extern const char* get_homedir(void)
{
    const char* homedir = NULL;

    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    assert(homedir);
    debug(3, "Determined ${HOME} to be %s\n", homedir);

    return homedir;
}

extern const char* creds_file_path(bool need_to_create, bool need_to_delete)
{
    assert(!(need_to_create && need_to_delete));

    const char* homedir = get_homedir();
    const char* account_subdir = "/.mod2ch";
    const char* account_file = "/.creds";

    size_t fullpathsize = strlen(homedir) + strlen(account_subdir) + strlen(account_file) + 1;

    char* fullpath = malloc(fullpathsize);
    assert(fullpath);

    memset(fullpath, 0, fullpathsize);
    strncpy(fullpath, homedir, strlen(homedir));
    strncat(fullpath, account_subdir, strlen(account_subdir));

    debug(3, "Determined credentials directory to be %s\n", fullpath);

    size_t res = 0;
    struct stat stat_buf = { 0 };
    if ((res = stat(fullpath, &stat_buf))) {
        debug(4, "Cannot access credentials directory %s (%s)\n", fullpath, strerror(errno));

        if (need_to_create) {
            if ((stat_buf.st_mode & S_IFMT) != S_IFDIR) {
                debug(4, "%s is not a directory, will try to create...\n", fullpath);
            }

            if ((res = mkdir(fullpath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))) {
                debug(1, "Cannot create credentials directory %s (%s)\n", fullpath, strerror(errno));
                free(fullpath);
                return NULL;
            }
        } else {
            // Cannot access a directory and need_to_create = false
            free(fullpath);
            return NULL;
        }
    }

    strncat(fullpath, account_file, strlen(account_file));
    debug(3, "Determined credentials file to be %s\n", fullpath);

    if ((res = stat(fullpath, &stat_buf))) {
        debug(4, "Cannot access credentials file %s (%s)\n", fullpath, strerror(errno));

        if (need_to_create) {
            if ((stat_buf.st_mode & S_IFMT) != S_IFREG) {
                debug(4, "%s is not a file, will try to create...\n", fullpath);
            }

            if ((res = open(fullpath, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) == -1) {
                debug(1, "Cannot create credentials file %s (%s)\n", fullpath, strerror(errno));
                free(fullpath);
                return NULL;
            }
        } else {
            // Cannot access a file and need_to_create = false
            free(fullpath);
            return NULL;
        }
    }

    if (need_to_delete) {
        if (unlink(fullpath)) {
            debug(1, "Was not able to remove %s (%s)\n", fullpath, strerror(errno));
        }
    }

    // copy result to stack
    const char* ret = fullpath;
    free(fullpath);
    return ret;
}

// Helper function for curl to write output.

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

// Helper function to init curl session.
// Curl object can be used later with custom per-function options.

static CURL* dvach_curl_init(struct curl_string* s, const char* cookie)
{
    CURL* curl = NULL;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    assert(curl);

    struct curl_slist* headers = NULL;

    char* user_agent = malloc(MAX_CRED_LENGTH);
    assert(user_agent);

    snprintf(user_agent, MAX_CRED_LENGTH - 2, "User-Agent: 2ch-mod/%f", 0.1);
    debug(3, "Will use %s\n", user_agent);

    headers = curl_slist_append(headers, user_agent);
    assert(headers);

    if (cookie) {
        headers = curl_slist_append(headers, cookie);
        debug(3, "Will use %s\n", cookie);
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, s);

    free(user_agent);
    return curl;
}

// Uses given credentials to login and obtain mod info to be later re-used.

extern ssize_t session_init(session_creds_t* creds, session_t* session)
{
    assert(creds);
    assert(session);

    char* postfields = malloc(MAX_CRED_LENGTH * 5);
    assert(postfields);

    ssize_t func_res = EXIT_FAILURE;

    if (!snprintf(postfields, (MAX_CRED_LENGTH * 5) - 2, "use_cookie=1&nick=%s&password=%s", creds->username, creds->password)) {
        debug(1, "Unable to populate postfields with credentials %s %s\n", creds->username, creds->password);
        free(postfields);
        return func_res;
    }

    debug(3, "Populated postfields %s\n", postfields);

    struct curl_string s = { .len = 0 };
    s.ptr = malloc(s.len + 1);
    assert(s.ptr);
    s.ptr[0] = '\0';

    CURL* curl = dvach_curl_init(&s, NULL);

    curl_easy_setopt(curl, CURLOPT_URL, "https://beta.2ch.hk/moder/login?json=1");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        debug(1, "Got curl_easy_perform err %s\n", curl_easy_strerror(res));
        goto cleanup;
    }

    size_t response_code = 0;

    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (res != CURLE_OK) {
        debug(1, "Got curl_easy_getinfo err %s\n", curl_easy_strerror(res));
        goto cleanup;
    }

    if (response_code == 200) {
        debug(3, "CURL result %s\n", s.ptr);

        cJSON* parse_result = NULL;
        if ((parse_result = cJSON_Parse(s.ptr)) == NULL) {

            const char* cjson_err = cJSON_GetErrorPtr();
            if (cjson_err) {
                debug(1, "cJSON err: %s", cjson_err);
            }
            cJSON_Delete(parse_result); // cjson checks for nullptr here
            goto cleanup;
        }

        session->creds = creds;
        dvach_popupate_session(session, parse_result);

        cJSON_Delete(parse_result); // cjson checks for nullptr here
    } else {
        debug(1, "CURL got http error code %zu result %s\n", response_code, s.ptr);
        goto cleanup;
    }

    free(postfields);
    free(s.ptr);
    curl_global_cleanup();
    func_res = EXIT_SUCCESS;
    return func_res;

cleanup:
    free(postfields);
    free(s.ptr);
    curl_easy_cleanup(curl);
    return func_res;
}

// Populates board info from catalog.
// Using mod cookie is optional.

extern board_t* fetch_board_info(session_t* session, const char* board_name)
{
    assert(session);
    assert(board_name);

    board_t* board = malloc(sizeof(board_t));
    assert(board);

    char* cookie = malloc(MAX_CRED_LENGTH);
    assert(cookie);
    snprintf(cookie, MAX_CRED_LENGTH - 2, "Cookie: %s", session->cookie);

    char* url = malloc(MAX_CRED_LENGTH);
    assert(url);

    struct curl_string s = { .len = 0 };
    s.ptr = malloc(s.len + 1);
    assert(s.ptr);
    s.ptr[0] = '\0';

    CURL* curl = dvach_curl_init(&s, cookie);

    if (!snprintf(url, MAX_CRED_LENGTH - 2, "https://2ch.hk/%s/catalog.json", board_name)) {
        debug(1, "Cannot assemble URL https://2ch.hk/%s/catalog.json", board_name);
        goto cleanup;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        debug(1, "Got curl_easy_perform err %s\n", curl_easy_strerror(res));
        goto cleanup;
    }

    size_t response_code = 0;

    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (res != CURLE_OK) {
        debug(1, "Got curl_easy_getinfo err %s\n", curl_easy_strerror(res));
        goto cleanup;
    }

    if (response_code == 200) {
        debug(3, "%s CURL result %s\n", url, s.ptr);

        cJSON* parse_result = NULL;
        if ((parse_result = cJSON_Parse(s.ptr)) == NULL) {

            const char* cjson_err = cJSON_GetErrorPtr();
            if (cjson_err) {
                debug(1, "cJSON err: %s", cjson_err);
            }
            cJSON_Delete(parse_result); // cjson checks for nullptr here
            goto cleanup;
        }

        dvach_popupate_board(board, parse_result);

        cJSON_Delete(parse_result); // cjson checks for nullptr here
    } else {
        debug(1, "CURL got HTTP error code %zu result %s\n", response_code, s.ptr);
        goto cleanup;
    }

    free(s.ptr);
    free(cookie);
    free(url);
    curl_global_cleanup();
    return board;

cleanup:
    free(board);
    free(s.ptr);
    free(cookie);
    free(url);
    curl_easy_cleanup(curl);
    return NULL;
}

// TODO
//extern post_t* fetch_all_posts_from_board(session_t* session, board_t* board) {}
//
//extern thread_t* fetch_thread_from_board(session_t* session, board_t* board) {}
//
//extern post_t* fetch_post_from_thread(session_t* session, thread_t* thread) {}
//
//extern file_t* fetch_file_from_post(session_t* session, post_t* post) {}

// Functions to populate allocated data structures
// Populates allocated session struct with moder info and session cookie.
static void dvach_popupate_session(session_t* session, cJSON* parse_result)
{
    // All nullptr checks have been performed by callee
    const cJSON* current_val = NULL;

    const cJSON* board = NULL;
    const cJSON* cookie = NULL;
    const cJSON* jid = NULL;
    const cJSON* mod = NULL;
    const cJSON* telegram_key = NULL;

    cookie = cJSON_GetObjectItemCaseSensitive(parse_result, "cookie");
    if (cJSON_IsString(cookie) && cookie->valuestring) {
        assert(strlen(cookie->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
        memcpy(&session->cookie, cookie->valuestring, strlen(cookie->valuestring));
        debug(3, "Populated cookie %s", session->cookie);
    }

    telegram_key = cJSON_GetObjectItemCaseSensitive(parse_result, "telegram_key");
    if (cJSON_IsString(telegram_key) && telegram_key->valuestring) {
        assert(strlen(telegram_key->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
        memcpy(&session->telegram_key, telegram_key->valuestring, strlen(telegram_key->valuestring));
        debug(3, "Populated telegram_key %s", session->telegram_key);
    }

    mod = cJSON_GetObjectItemCaseSensitive(parse_result, "moder");
    if (cJSON_IsObject(mod)) {

        cJSON_ArrayForEach(current_val, mod)
        {
            // Iterate over keys
            const char* current_key = current_val->string;

            if (current_key) {
                debug(4, "Processing %s\n", current_key);
                const char* trace = cJSON_Print(current_val);
                debug(5, "%s\n", trace);

                // Start comparing keys
                if (strcmp(current_key, "Num") == 0) {
                    if (cJSON_IsNumber(current_val) && current_val->valueint) {
                        session->moder.num = current_val->valueint;
                        debug(3, "Populated Num %zu", session->moder.num);
                    }
                } else if (strcmp(current_key, "Nick") == 0) {
                    if (cJSON_IsString(current_val) && current_val->valuestring) {
                        assert(strlen(current_val->valuestring) < MAX_CRED_LENGTH);
                        memcpy(&session->moder.nick, current_val->valuestring, strlen(current_val->valuestring));
                        debug(3, "Populated Nick %s", session->moder.nick);
                    }
                } else if (strcmp(current_key, "Level") == 0) {
                    if (cJSON_IsNumber(current_val) && current_val->valueint) {
                        session->moder.level = current_val->valueint;
                        debug(3, "Populated Level %zu", session->moder.level);
                    }
                } else if (strcmp(current_key, "RequestMessage") == 0) {
                    if (cJSON_IsString(current_val) && current_val->valuestring) {
                        assert(strlen(current_val->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
                        memcpy(&session->moder.request_message, current_val->valuestring, strlen(current_val->valuestring));
                        debug(3, "Populated RequestMessage %s", session->moder.request_message);
                    }
                } else if (strcmp(current_key, "Email") == 0) {
                    if (cJSON_IsString(current_val) && current_val->valuestring) {
                        assert(strlen(current_val->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
                        memcpy(&session->moder.email, current_val->valuestring, strlen(current_val->valuestring));
                        debug(3, "Populated Email %s", session->moder.email);
                    }
                } else if (strcmp(current_key, "TelegramID") == 0) {
                    if (cJSON_IsNumber(current_val) && current_val->valueint) {
                        session->moder.telegram_id = current_val->valueint;
                        debug(3, "Populated TelegramID %zu", session->moder.telegram_id);
                    }
                } else if (strcmp(current_key, "TelegramUsername") == 0) {
                    if (cJSON_IsString(current_val) && current_val->valuestring) {
                        assert(strlen(current_val->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
                        memcpy(&session->moder.telegram_username, current_val->valuestring, strlen(current_val->valuestring));
                        debug(3, "Populated TelegramUsername %s", session->moder.telegram_username);
                    }
                } else if (strcmp(current_key, "Jids") == 0) {
                    if (cJSON_IsArray(current_val)) {
                        size_t curr_iteration = 0;

                        cJSON_ArrayForEach(jid, current_val)
                        {
                            size_t curr_arr_element = curr_iteration * MAX_CRED_LENGTH;

                            if (cJSON_IsString(jid) && jid->valuestring) {
                                assert(strlen(jid->valuestring) < MAX_CRED_LENGTH);
                                assert(curr_arr_element < MAX_CRED_LENGTH * MAX_NUM_OF_JIDS);

                                memcpy(&session->moder.jids[curr_iteration], jid->valuestring, strlen(jid->valuestring));
                                const char null = '\0'; // add null terminator
                                memcpy(&session->moder.jids[curr_iteration] + strlen(jid->valuestring), &null, 1);

                                debug(3, "Populated Jid #%zu (%zu) %s", curr_iteration, curr_arr_element, (char*)session->moder.jids + curr_arr_element);
                                ++curr_iteration;
                            }
                        }
                    }
                } else if (strcmp(current_key, "Comment") == 0) {
                    if (cJSON_IsString(current_val) && current_val->valuestring) {
                        assert(strlen(current_val->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
                        memcpy(&session->moder.comment, current_val->valuestring, strlen(current_val->valuestring));
                        debug(3, "Populated Comment %s", session->moder.comment);
                    }
                } else if (strcmp(current_key, "Boards") == 0) {
                    if (cJSON_IsArray(current_val)) {
                        size_t curr_iteration = 0;

                        cJSON_ArrayForEach(board, current_val)
                        {
                            size_t curr_arr_element = curr_iteration * MAX_BOARD_NAME_LENGTH;

                            if (cJSON_IsString(board) && board->valuestring) {
                                assert(strlen(board->valuestring) < MAX_BOARD_NAME_LENGTH);
                                assert(curr_arr_element < MAX_BOARD_NAME_LENGTH * MAX_NUM_OF_BOARDS);

                                memcpy(&session->moder.boards[curr_iteration], board->valuestring, strlen(board->valuestring));
                                const char null = '\0'; // add null terminator
                                memcpy(&session->moder.boards[curr_iteration] + strlen(board->valuestring), &null, 1);

                                debug(3, "Populated Board #%zu (%zu) %s", curr_iteration, curr_arr_element, (char*)session->moder.boards + curr_arr_element);
                                ++curr_iteration;
                            }
                        }
                    }
                } else if (strcmp(current_key, "PublicLog") == 0) {
                    session->moder.public_log = false;
                    if (cJSON_IsBool(current_val)) {
                        if (cJSON_IsTrue(current_val)) {
                            session->moder.public_log = true;
                        } else if (cJSON_IsFalse(current_val)) {
                        } else {
                            debug(2, "Got unknown PublicLog %s", cJSON_Print(current_val));
                        }
                        debug(3, "Populated PublicLog %u", session->moder.public_log);
                    }
                } else if (strcmp(current_key, "LastLogin") == 0) {
                    if (cJSON_IsNumber(current_val) && current_val->valueint) {
                        session->moder.last_login = current_val->valueint;
                        debug(3, "Populated LastLogin %zu", session->moder.last_login);
                    }
                } else if (strcmp(current_key, "LastAction") == 0) {
                    if (cJSON_IsNumber(current_val) && current_val->valueint) {
                        session->moder.last_action = current_val->valueint;
                        debug(3, "Populated LastAction %zu", session->moder.last_action);
                    }
                } else {
                    debug(2, "Got unknown key while iterating %s", current_key);
                }
            }
        }
    }
}

// Populates allocated session struct with board info from catalog.
static void dvach_popupate_board(board_t* board, cJSON* parse_result)
{
    // All nullptr checks have been performed by callee
    const cJSON* current_val = NULL;

    const cJSON* name = NULL;
    const cJSON* verbose_name = NULL;
    const cJSON* bump_limit = NULL;
    const cJSON* file = NULL;
    const cJSON* file_obj = NULL;
    const cJSON* threads = NULL;
    const cJSON* thread_obj = NULL;

    name = cJSON_GetObjectItemCaseSensitive(parse_result, "Board");
    if (cJSON_IsString(name) && name->valuestring) {
        assert(strlen(name->valuestring) < MAX_BOARD_NAME_LENGTH);
        memcpy(&board->name, name->valuestring, strlen(name->valuestring));
        debug(3, "Populated board name %s", board->name);
    }

    verbose_name = cJSON_GetObjectItemCaseSensitive(parse_result, "BoardName");
    if (cJSON_IsString(verbose_name) && verbose_name->valuestring) {
        assert(strlen(verbose_name->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
        memcpy(&board->verbose_name, verbose_name->valuestring, strlen(verbose_name->valuestring));
        debug(3, "Populated board verbose_name %s", board->verbose_name);
    }

    bump_limit = cJSON_GetObjectItemCaseSensitive(parse_result, "bump_limit");
    if (cJSON_IsNumber(bump_limit) && bump_limit->valueint) {
        board->bump_limit = bump_limit->valueint;
        debug(3, "Populated board bumplimit %zu", board->bump_limit);
    }

    threads = cJSON_GetObjectItemCaseSensitive(parse_result, "threads");
    if (cJSON_IsArray(threads)) {
        size_t curr_iteration = 0;

        cJSON_ArrayForEach(thread_obj, threads)
        {
            if (cJSON_IsObject(thread_obj)) {

                cJSON_ArrayForEach(current_val, thread_obj)
                {
                    // Iterate over keys
                    const char* current_key = current_val->string;

                    if (current_key) {
                        debug(4, "Processing %s[%zu]\n", current_key, curr_iteration);
                        char* trace = cJSON_Print(current_val);
                        debug(5, "%s\n", trace);

                        assert(curr_iteration < MAX_NUM_OF_THREADS);

                        // Start populating threads on the board
                        // Start comparing keys
                        if (strcmp(current_key, "banned") == 0) {
                            if (cJSON_IsNumber(current_val) && current_val->valueint) {
                                board->thread[curr_iteration].banned = current_val->valueint ? true : false;
                                debug(3, "Got banned %u", board->thread[curr_iteration].banned);
                            }
                        } else if (strcmp(current_key, "closed") == 0) {
                            if (cJSON_IsNumber(current_val) && current_val->valueint) {
                                board->thread[curr_iteration].closed = current_val->valueint ? true : false;
                                debug(3, "Got closed %u", board->thread[curr_iteration].closed);
                            }
                        } else if (strcmp(current_key, "comment") == 0) {
                            if (cJSON_IsString(current_val) && current_val->valuestring) {
                                assert(strlen(current_val->valuestring) < 90 * MAX_ARBITRARY_CHAR_LENGTH);
                                memcpy(&board->thread[curr_iteration].comment, current_val->valuestring, strlen(current_val->valuestring));
                                debug(3, "Got comment %s", board->thread[curr_iteration].comment);
                            }
                        } else if (strcmp(current_key, "date") == 0) {
                            if (cJSON_IsString(current_val) && current_val->valuestring) {
                                assert(strlen(current_val->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
                                memcpy(&board->thread[curr_iteration].date, current_val->valuestring, strlen(current_val->valuestring));
                                debug(3, "Got date %s", board->thread[curr_iteration].date);
                            }
                        } else if (strcmp(current_key, "email") == 0) {
                            if (cJSON_IsString(current_val) && current_val->valuestring) {
                                assert(strlen(current_val->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
                                memcpy(&board->thread[curr_iteration].email, current_val->valuestring, strlen(current_val->valuestring));
                                debug(3, "Got email %s", board->thread[curr_iteration].email);
                            }
                        } else if (strcmp(current_key, "files") == 0) {
                            if (cJSON_IsArray(current_val)) {
                                size_t file_curr_iteration = 0;

                                cJSON_ArrayForEach(file_obj, current_val)
                                {
                                    debug(4, "Processing file array for %zu[%zu]\n", curr_iteration, file_curr_iteration);
                                    if (cJSON_IsObject(file_obj)) {

                                        cJSON_ArrayForEach(file, file_obj)
                                        {
                                            // Iterate over keys
                                            const char* file_current_key = file->string;

                                            if (file_current_key) {
                                                assert(file_curr_iteration < MAX_NUM_OF_FILES);

                                                // Start populating files on the thread
                                                // Start comparing keys
                                                if (strcmp(file_current_key, "path") == 0) {
                                                    if (cJSON_IsString(file) && file->valuestring) {
                                                        assert(strlen(file->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
                                                        memcpy(&board->thread[curr_iteration].file[file_curr_iteration].path, file->valuestring, strlen(file->valuestring));
                                                        debug(3, "Got file path %s\n", board->thread[curr_iteration].file[file_curr_iteration].path);
                                                    }
                                                } else if (strcmp(file_current_key, "displayname") == 0) {
                                                    if (cJSON_IsString(file) && file->valuestring) {
                                                        assert(strlen(file->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
                                                        memcpy(&board->thread[curr_iteration].file[file_curr_iteration].displayname, file->valuestring, strlen(file->valuestring));
                                                        debug(3, "Got file displayname %s\n", board->thread[curr_iteration].file[file_curr_iteration].displayname);
                                                    }
                                                } else if (strcmp(file_current_key, "fullname") == 0) {
                                                    if (cJSON_IsString(file) && file->valuestring) {
                                                        assert(strlen(file->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
                                                        memcpy(&board->thread[curr_iteration].file[file_curr_iteration].fullname, file->valuestring, strlen(file->valuestring));
                                                        debug(3, "Got file fullname %s\n", board->thread[curr_iteration].file[file_curr_iteration].fullname);
                                                    }
                                                } else if (strcmp(file_current_key, "md5") == 0) {
                                                    if (cJSON_IsString(file) && file->valuestring) {
                                                        assert(strlen(file->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
                                                        memcpy(&board->thread[curr_iteration].file[file_curr_iteration].md5, file->valuestring, strlen(file->valuestring));
                                                        debug(3, "Got file md5 %s\n", board->thread[curr_iteration].file[file_curr_iteration].md5);
                                                    }
                                                } else if (strcmp(file_current_key, "name") == 0) {
                                                    if (cJSON_IsString(file) && file->valuestring) {
                                                        assert(strlen(file->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
                                                        memcpy(&board->thread[curr_iteration].file[file_curr_iteration].name, file->valuestring, strlen(file->valuestring));
                                                        debug(3, "Got file name %s\n", board->thread[curr_iteration].file[file_curr_iteration].name);
                                                    }
                                                } else if (strcmp(file_current_key, "thumbnail") == 0) {
                                                    if (cJSON_IsString(file) && file->valuestring) {
                                                        assert(strlen(file->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
                                                        memcpy(&board->thread[curr_iteration].file[file_curr_iteration].thumbnail, file->valuestring, strlen(file->valuestring));
                                                        debug(3, "Got file thumbnail %s\n", board->thread[curr_iteration].file[file_curr_iteration].thumbnail);
                                                    }
                                                } else if (strcmp(file_current_key, "height") == 0) {
                                                    if (cJSON_IsNumber(file) && file->valueint) {
                                                        board->thread[curr_iteration].file[file_curr_iteration].height = file->valueint;
                                                        debug(3, "Got file height %zu\n", board->thread[curr_iteration].file[file_curr_iteration].height);
                                                    }
                                                } else if (strcmp(file_current_key, "width") == 0) {
                                                    if (cJSON_IsNumber(file) && file->valueint) {
                                                        board->thread[curr_iteration].file[file_curr_iteration].width = file->valueint;
                                                        debug(3, "Got file width %zu\n", board->thread[curr_iteration].file[file_curr_iteration].width);
                                                    }
                                                } else if (strcmp(file_current_key, "height") == 0) {
                                                    if (cJSON_IsNumber(file) && file->valueint) {
                                                        board->thread[curr_iteration].file[file_curr_iteration].height = file->valueint;
                                                        debug(3, "Got file height %zu\n", board->thread[curr_iteration].file[file_curr_iteration].height);
                                                    }
                                                } else if (strcmp(file_current_key, "size") == 0) {
                                                    if (cJSON_IsNumber(file) && file->valueint) {
                                                        board->thread[curr_iteration].file[file_curr_iteration].size = file->valueint;
                                                        debug(3, "Got file size %zu\n", board->thread[curr_iteration].file[file_curr_iteration].size);
                                                    }
                                                } else if (strcmp(file_current_key, "tn_height") == 0) {
                                                    if (cJSON_IsNumber(file) && file->valueint) {
                                                        board->thread[curr_iteration].file[file_curr_iteration].tn_height = file->valueint;
                                                        debug(3, "Got file tn_height %zu\n", board->thread[curr_iteration].file[file_curr_iteration].tn_height);
                                                    }
                                                } else if (strcmp(file_current_key, "tn_width") == 0) {
                                                    if (cJSON_IsNumber(file) && file->valueint) {
                                                        board->thread[curr_iteration].file[file_curr_iteration].tn_width = file->valueint;
                                                        debug(3, "Got file tn_width %zu\n", board->thread[curr_iteration].file[file_curr_iteration].tn_width);
                                                    }
                                                } else if (strcmp(file_current_key, "type") == 0) {
                                                    if (cJSON_IsNumber(file) && file->valueint) {
                                                        board->thread[curr_iteration].file[file_curr_iteration].type = file->valueint;
                                                        debug(3, "Got file type %zu\n", board->thread[curr_iteration].file[file_curr_iteration].type);
                                                    }
                                                } else {
                                                    debug(2, "Skipping file key %s\n", file_current_key);
                                                }
                                            }
                                        }
                                    }
                                    ++file_curr_iteration;
                                }
                            }
                        } else if (strcmp(current_key, "lasthit") == 0) {
                            if (cJSON_IsNumber(current_val) && current_val->valueint) {
                                board->thread[curr_iteration].lasthit = current_val->valueint;
                                debug(3, "Got lasthit %zu\n", board->thread[curr_iteration].lasthit);
                            }
                        } else if (strcmp(current_key, "posts_count") == 0) {
                            if (cJSON_IsNumber(current_val) && current_val->valueint) {
                                board->thread[curr_iteration].posts_count = current_val->valueint;
                                debug(3, "Got posts_count %zu\n", board->thread[curr_iteration].posts_count);
                            }
                        } else if (strcmp(current_key, "num") == 0) {
                            if (cJSON_IsString(current_val) && current_val->valuestring) {
                                board->thread[curr_iteration].num = atoi(current_val->valuestring);
                                debug(3, "Got num %zu\n", board->thread[curr_iteration].num);
                            }
                        } else if (strcmp(current_key, "sticky") == 0) {
                            if (cJSON_IsNumber(current_val) && current_val->valueint) {
                                board->thread[curr_iteration].sticky = current_val->valueint ? true : false;
                                debug(3, "Got sticky %u\n", board->thread[curr_iteration].sticky);
                            }
                        } else if (strcmp(current_key, "op") == 0) {
                            if (cJSON_IsNumber(current_val) && current_val->valueint) {
                                board->thread[curr_iteration].op = current_val->valueint ? true : false;
                                debug(3, "Got op %u\n", board->thread[curr_iteration].op);
                            }
                        } else if (strcmp(current_key, "timestamp") == 0) {
                            if (cJSON_IsNumber(current_val) && current_val->valueint) {
                                board->thread[curr_iteration].timestamp = current_val->valueint;
                                debug(3, "Got timestamp %zu\n", board->thread[curr_iteration].timestamp);
                            }
                        } else if (strcmp(current_key, "parent") == 0) {
                            if (cJSON_IsNumber(current_val) && current_val->valueint) {
                                board->thread[curr_iteration].parent = current_val->valueint;
                                debug(3, "Got parent %zu\n", board->thread[curr_iteration].parent);
                            }
                        } else if (strcmp(current_key, "subject") == 0) {
                            if (cJSON_IsString(current_val) && current_val->valuestring) {
                                assert(strlen(current_val->valuestring) < MAX_ARBITRARY_CHAR_LENGTH);
                                memcpy(&board->thread[curr_iteration].subject, current_val->valuestring, strlen(current_val->valuestring));
                                debug(3, "Got subject %s\n", board->thread[curr_iteration].subject);
                            }
                        } else {
                            debug(2, "Skipping thread key %s\n", current_key);
                        }
                    }
                }
            }
            ++curr_iteration;
        }
    }
}
