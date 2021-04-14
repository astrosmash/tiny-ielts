extern size_t check_local_account(void)
{
    size_t res = 0;
    const char* homedir = NULL;

    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    assert(homedir);
    debug("homedir is %s\n", homedir);

    const char* dvach_subdir = "/.mod2ch";
    const char* dvach_account_file = "/.creds";

    size_t fullpathsize = strlen(homedir) + strlen(dvach_subdir) + strlen(dvach_account_file) + 2;
    char* fullpath = malloc(fullpathsize);
    assert(fullpath);

    memset(fullpath, 0, fullpathsize);

    strncpy(fullpath, homedir, strlen(homedir));

    struct stat stat_buf = { 0 };

    strncat(fullpath, dvach_subdir, strlen(dvach_subdir));
    debug("local folder with credentials is %s\n", fullpath);

    if ((res = stat(fullpath, &stat_buf))) {
        debug("cannot access local folder with credentials %s (%d)\n", fullpath, errno);
        return res;
    }

    if ((stat_buf.st_mode & S_IFMT) != S_IFDIR) {
        res = -1;
        debug("%s is not a directory\n", fullpath);
        return res;
    }

    strncat(fullpath, dvach_account_file, strlen(dvach_account_file));
    debug("local file with credentials is %s\n", fullpath);

    if ((res = stat(fullpath, &stat_buf))) {
        debug("cannot access local file with credentials %s (%d)\n", fullpath, errno);
        return res;
    }

    if ((stat_buf.st_mode & S_IFMT) != S_IFREG) {
        res = -1;
        debug("%s is not a file\n", fullpath);
        return res;
    }

    return res;
}

extern size_t check_local_file(const char* path)
{
    const char* check_path = path;
    assert(check_path);
    debug("checking path %s\n", check_path);

    size_t stat_res = 0;
    struct stat stat_buf = { 0 };

    if ((stat_res = stat(check_path, &stat_buf))) {
        debug("%d\n", errno);
    }

    return stat_res;
}

static size_t curl_write_func(void* ptr, size_t size, size_t nmemb, struct curl_string* s)
{
    assert(ptr);
    assert(s);

    size_t total_size = size * nmemb;
    size_t new_len = s->len + total_size;

    s->ptr = realloc(s->ptr, new_len + 1);
    assert(s->ptr);

    memcpy(s->ptr + s->len, ptr, total_size);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return total_size;
}

extern ssize_t session_init(session_creds_t* creds, session_t* session)
{
    assert(creds);
    assert(session);

    char* postfields = malloc(MAX_CRED_LENGTH * 5);
    assert(postfields);

    ssize_t func_res = EXIT_SUCCESS;

    if (!snprintf(postfields, (MAX_CRED_LENGTH * 3) - 2, "use_cookie=1&nick=%s&password=%s", creds->username, creds->password)) {
        debug("cannot populate postfields with %s %s\n", creds->username, creds->password);
        func_res = EXIT_FAILURE;
        return func_res;
    }

    debug("called with %s\n", postfields);

    CURL* curl = NULL;
    CURLcode res = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    assert(curl);

    struct curl_string s = { .len = 0 };
    s.ptr = malloc(s.len + 1);
    assert(s.ptr);
    s.ptr[0] = '\0';

    curl_easy_setopt(curl, CURLOPT_URL, "https://beta.2ch.hk/moder/login?json=1");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        debug("got curl_easy_perform err %s\n", curl_easy_strerror(res));
        func_res = EXIT_FAILURE;

        curl_easy_cleanup(curl);
        free(s.ptr);
        s.ptr = NULL;

        return func_res;
    }

    size_t response_code = 0;

    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (res != CURLE_OK) {
        debug("got curl_easy_getinfo err %s\n", curl_easy_strerror(res));
        func_res = EXIT_FAILURE;

        curl_easy_cleanup(curl);
        free(s.ptr);
        s.ptr = NULL;

        return func_res;
    }

    if (response_code == 200) {
        debug("CURL result %s\n", s.ptr);

        const cJSON* cookie = NULL;
        const cJSON* telegram_key = NULL;
        const cJSON* mod = NULL;
        const cJSON* current_val = NULL;
        const cJSON* jid = NULL;
        const cJSON* board = NULL;

        cJSON* parse_result = NULL;
        if ((parse_result = cJSON_Parse(s.ptr)) == NULL) {
            func_res = EXIT_FAILURE;

            const char* cjson_err = cJSON_GetErrorPtr();
            if (cjson_err) {
                debug("cJSON err: %s", cjson_err);
            }
            cJSON_Delete(parse_result); // cjson checks for nullptr here

            return func_res;
        }

        session->creds = creds;

        cookie = cJSON_GetObjectItemCaseSensitive(parse_result, "cookie");
        if (cJSON_IsString(cookie) && cookie->valuestring) {
            assert(strlen(cookie->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
            memcpy(&session->cookie, cookie->valuestring, strlen(cookie->valuestring)); // no strncpy as dst is a stack array
            debug("got cookie %s", session->cookie);
        }

        telegram_key = cJSON_GetObjectItemCaseSensitive(parse_result, "telegram_key");
        if (cJSON_IsString(telegram_key) && telegram_key->valuestring) {
            assert(strlen(telegram_key->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
            memcpy(&session->telegram_key, telegram_key->valuestring, strlen(telegram_key->valuestring)); // no strncpy as dst is a stack array
            debug("got telegram_key %s", session->telegram_key);
        }

        mod = cJSON_GetObjectItemCaseSensitive(parse_result, "moder");
        if (cJSON_IsObject(mod)) {

            cJSON_ArrayForEach(current_val, mod)
            {
                // Iterate over keys
                const char* current_key = current_val->string;

                if (current_key) {
                    debug("processing %s\n", current_key);
                    // Get current value
                    char* trace = cJSON_Print(current_val);
                    debug("%s\n", trace);

                    // Start comparing keys
                    if (strcmp(current_key, "Num") == 0) {
                        if (cJSON_IsNumber(current_val) && current_val->valueint) {
                            session->moder.num = current_val->valueint;
                            debug("got Num %zu", session->moder.num);
                        }
                    } else if (strcmp(current_key, "Nick") == 0) {
                        if (cJSON_IsString(current_val) && current_val->valuestring) {
                            assert(strlen(current_val->valuestring) < MAX_CRED_LENGTH);
                            memcpy(&session->moder.nick, current_val->valuestring, strlen(current_val->valuestring)); // no strncpy as dst is a stack array
                            debug("got Nick %s", session->moder.nick);
                        }
                    } else if (strcmp(current_key, "Level") == 0) {
                        if (cJSON_IsNumber(current_val) && current_val->valueint) {
                            session->moder.level = current_val->valueint;
                            debug("got Level %zu", session->moder.level);
                        }
                    } else if (strcmp(current_key, "RequestMessage") == 0) {
                        if (cJSON_IsString(current_val) && current_val->valuestring) {
                            assert(strlen(current_val->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
                            memcpy(&session->moder.request_message, current_val->valuestring, strlen(current_val->valuestring)); // no strncpy as dst is a stack array
                            debug("got RequestMessage %s", session->moder.request_message);
                        }
                    } else if (strcmp(current_key, "Email") == 0) {
                        if (cJSON_IsString(current_val) && current_val->valuestring) {
                            assert(strlen(current_val->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
                            memcpy(&session->moder.email, current_val->valuestring, strlen(current_val->valuestring)); // no strncpy as dst is a stack array
                            debug("got Email %s", session->moder.email);
                        }
                    } else if (strcmp(current_key, "TelegramID") == 0) {
                        if (cJSON_IsNumber(current_val) && current_val->valueint) {
                            session->moder.telegram_id = current_val->valueint;
                            debug("got TelegramID %zu", session->moder.telegram_id);
                        }
                    } else if (strcmp(current_key, "TelegramUsername") == 0) {
                        if (cJSON_IsString(current_val) && current_val->valuestring) {
                            assert(strlen(current_val->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
                            memcpy(&session->moder.telegram_username, current_val->valuestring, strlen(current_val->valuestring)); // no strncpy as dst is a stack array
                            debug("got TelegramUsername %s", session->moder.telegram_username);
                        }
                    } else if (strcmp(current_key, "Jids") == 0) {
                        if (cJSON_IsArray(current_val)) {
                            size_t curr_iteration = 0;
                            size_t curr_arr_element = curr_iteration * MAX_CRED_LENGTH;

                            cJSON_ArrayForEach(jid, current_val)
                            {
                                if (cJSON_IsString(jid) && jid->valuestring) {
                                    assert(strlen(jid->valuestring) < MAX_CRED_LENGTH);
                                    assert(curr_arr_element < MAX_CRED_LENGTH * MAX_NUM_OF_JIDS);

                                    memcpy(&session->moder.jids + curr_arr_element, jid->valuestring, strlen(jid->valuestring));
                                    debug("got Jid #%zu %s", curr_iteration, (char*)session->moder.jids + curr_arr_element);
                                    ++curr_iteration;
                                }
                            }
                        }
                    } else if (strcmp(current_key, "Comment") == 0) {
                        if (cJSON_IsString(current_val) && current_val->valuestring) {
                            assert(strlen(current_val->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
                            memcpy(&session->moder.comment, current_val->valuestring, strlen(current_val->valuestring)); // no strncpy as dst is a stack array
                            debug("got Comment %s", session->moder.comment);
                        }
                    } else if (strcmp(current_key, "Boards") == 0) {
                        if (cJSON_IsArray(current_val)) {
                            size_t curr_iteration = 0;
                            size_t curr_arr_element = curr_iteration * MAX_BOARD_NAME_LENGTH;

                            cJSON_ArrayForEach(board, current_val)
                            {
                                if (cJSON_IsString(board) && board->valuestring) {
                                    assert(strlen(board->valuestring) < MAX_BOARD_NAME_LENGTH);
                                    assert(curr_arr_element < MAX_BOARD_NAME_LENGTH * MAX_NUM_OF_BOARDS);

                                    memcpy(&session->moder.boards + curr_arr_element, board->valuestring, strlen(board->valuestring));
                                    debug("got Board #%zu %s", curr_iteration, (char*)session->moder.boards + curr_arr_element);
                                    ++curr_iteration;
                                }
                            }
                        }
                    } else if (strcmp(current_key, "PublicLog") == 0) {
                        if (cJSON_IsBool(current_val)) {
                            if (cJSON_IsTrue(current_val)) {
                                session->moder.public_log = true;
                            } else if (cJSON_IsFalse(current_val)) {
                                session->moder.public_log = false;
                            } else {
                                debug("got unknown PublicLog %s", cJSON_Print(current_val));
                            }
                            debug("got PublicLog %u", session->moder.public_log);
                        }
                    } else if (strcmp(current_key, "LastLogin") == 0) {
                        if (cJSON_IsNumber(current_val) && current_val->valueint) {
                            session->moder.last_login = current_val->valueint;
                            debug("got LastLogin %zu", session->moder.last_login);
                        }
                    } else if (strcmp(current_key, "LastAction") == 0) {
                        if (cJSON_IsNumber(current_val) && current_val->valueint) {
                            session->moder.last_action = current_val->valueint;
                            debug("got LastAction %zu", session->moder.last_action);
                        }
                    } else {
                        func_res = EXIT_FAILURE;

                        debug("got unknown key while iterating %s", current_key);
                        cJSON_Delete(parse_result); // cjson checks for nullptr here

                        return func_res;
                    }
                }
            }
        }

        cJSON_Delete(parse_result); // cjson checks for nullptr here
    } else {
        debug("CURL got http error code %zu result %s\n", response_code, s.ptr);
    }

    curl_global_cleanup();

    return func_res;
}
