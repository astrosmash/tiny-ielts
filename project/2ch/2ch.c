// Authentication
bool populate_session_from_file(session_t* session)
{
    assert(session);

    char* fullpath = config_file_path(NEED_TO_CHECK);
    assert(fullpath);

    debug(3, "Reading credentials from %s\n", fullpath);
    const char* opmode = "r";
    FILE* file = NULL;

    if ((file = fopen(fullpath, opmode)) == NULL) {
        debug(1, "fopen(%s) no file\n", fullpath);
        safe_free((void**)&fullpath);
        return false;
    }

    size_t buf_size = 1024;
    char* buf = (char*)malloc_memset(buf_size);
    size_t ret = fread(buf, sizeof(char), buf_size, file);
    fclose(file);

    debug(4, "fread(%s) %zu bytes\n", fullpath, ret);

    if (ret < 32) {
        // no content was read in file or it seems corrupt
        // remove it so it gets re-created on next app launch
        debug(1, "No content was read or auth file corrupt? Removing %s\n", fullpath);
        fullpath = config_file_path(NEED_TO_DELETE); // remove file
        safe_free((void**)&buf);
        safe_free((void**)&fullpath);
        return false;
    }

    char* strtok_saveptr = NULL;
    char* line = strtok_r(buf, "\n", &strtok_saveptr);
    while (line != NULL) {
        if (sscanf(line, "cookie = %99s\n", session->cookie) == 1) { // read no more than 99 bytes
            debug(3, "Scanned cookie %s\n", session->cookie);
        }
        if (sscanf(line, "username = %99s\n", session->creds->username) == 1) {
            debug(3, "Scanned username %s\n", session->creds->username);
        }
        if (sscanf(line, "password = %99s\n", session->creds->password) == 1) {
            debug(3, "Scanned password %s\n", session->creds->password);
        }

        for (size_t i = 0; i <= MAX_NUM_OF_BOARDS; ++i) {
            if (!strlen(session->moder.boards[i]) && sscanf(line, "board = %15s\n", session->moder.boards[i]) == 1) {
                debug(3, "Scanned board %s\n", session->moder.boards[i]);
                break;
            }
        }
        line = strtok_r(NULL, "\n", &strtok_saveptr);
    }

    debug(3, "Populated credentials cookie: %s\n", session->cookie);
    safe_free((void**)&buf);
    safe_free((void**)&fullpath);
    return true;
}

// Refresh cookie and other contents in the file using session data
bool populate_file_from_session(session_creds_t* creds, session_t* session)
{
    assert(creds);
    assert(session);

    debug(3, "Trying to download CSV with Key %s GID %s\n", creds->username, creds->password);

    ssize_t session_res = 0;
    if ((session_res = session_init(creds, session))) {
        debug(1, "Was not able to download CSV with Key %s GID %s (%zd)\n", creds->username, creds->password, session_res);
        return false;
    }

    // No file was present and we are authenticating. Create a file
    char* fullpath = config_file_path(NEED_TO_CREATE);
    assert(fullpath);
    debug(3, "Writing credentials to %s\n", fullpath);

    const char* mode = "w";
    FILE* file = NULL;

    if ((file = fopen(fullpath, mode)) == NULL) {
        debug(1, "fopen(%s) cannot open file\n", fullpath);
        safe_free((void**)&fullpath);
        return false;
    }

    // Write to credentials file
    char* content = malloc_memset(MAX_ARBITRARY_CHAR_LENGTH);
    snprintf(content, MAX_ARBITRARY_CHAR_LENGTH - 2, "cookie = %s\nusername = %s\npassword = %s\n", session->cookie, session->creds->username, session->creds->password);

    for (size_t i = 0; i <= MAX_NUM_OF_BOARDS; ++i) {
        if (strlen(session->moder.boards[i])) {
            char* add = malloc_memset(MAX_ARBITRARY_CHAR_LENGTH);
            snprintf(add, MAX_ARBITRARY_CHAR_LENGTH, "board = %s\n", session->moder.boards[i]);
            strncat(content, add, strlen(add));
            safe_free((void**)&add);
        }
    }

    size_t ret = fwrite(content, sizeof(char), strlen(content), file);
    if (!ret) {
        debug(1, "fwrite(%s) cannot write to file\n", fullpath);
        fclose(file);
        safe_free((void**)&content);
        safe_free((void**)&fullpath);
        return false;
    }

    fclose(file);
    safe_free((void**)&content);
    safe_free((void**)&fullpath);
    return true;
}

// Uses given credentials to login and obtain mod info to be later re-used.
ssize_t session_init(session_creds_t* creds, session_t* session)
{
    assert(creds);
    assert(session);

    struct curl_string s = { .len = 0 };
    s.ptr = malloc_memset(s.len + 1);
    s.ptr[0] = '\0';

    char* url = malloc_memset(MAX_CRED_LENGTH);
    if (!snprintf(url, MAX_CRED_LENGTH - 2, "https://docs.google.com/spreadsheets/d/%s/export?gid=%s&format=csv", session->creds->username, session->creds->password)) {
        debug(1, "Cannot assemble URL https://docs.google.com/spreadsheets/d/%s/export?gid=%s&format=csv", session->creds->username, session->creds->password);
        goto cleanup;
    }
    debug(1, "Populated URL %s", url);

    bool res = submit_curl_task(url, NULL, &s, NULL);
    if (res) {
        cJSON* parse_result = NULL;
        if ((parse_result = cJSON_Parse(s.ptr)) == NULL) {

            const char* cjson_err = cJSON_GetErrorPtr();
            if (cjson_err) {
                debug(1, "cJSON err: %s", cjson_err);
            }
            cJSON_Delete(parse_result); // cjson checks for nullptr here
            goto cleanup;
        }

//        session->creds = creds;
//        dvach_populate_session(session, parse_result);

        cJSON_Delete(parse_result); // cjson checks for nullptr here
    } else {
        debug(1, "submit_curl_task failed: %u ", res);
        goto cleanup;
    }

    safe_free((void**)&url);
    safe_free((void**)&s.ptr);
    return EXIT_SUCCESS;

cleanup:
    safe_free((void**)&url);
    safe_free((void**)&s.ptr);
    return EXIT_FAILURE;
}

// Not as moder:
// Populates board info from catalog.
// Using mod cookie is optional.
// As moder:
// Populates board info from mod api.
// Using mod cookie is mandatory.

void* fetch_board_info(session_t* session, const char* board_name, bool as_moder)
{
    assert(session);
    assert(board_name);

    void* result = NULL;

    if (as_moder) {
        board_as_moder_t* board = malloc_memset(sizeof(board_as_moder_t));
        result = board;
    } else {
        board_t* board = malloc_memset(sizeof(board_t));
        result = board;
    }

    char* cookie = malloc_memset(MAX_CRED_LENGTH);
    char* url = malloc_memset(MAX_CRED_LENGTH);

    struct curl_string s = { .len = 0 };
    s.ptr = malloc_memset(s.len + 1);
    s.ptr[0] = '\0';

    if (!snprintf(cookie, MAX_CRED_LENGTH - 2, "Cookie: moder=%s", session->cookie)) {
        debug(1, "Cannot assemble Cookie header moder=%s", session->cookie);
        goto cleanup;
    }

    if (as_moder) {
        if (!snprintf(url, MAX_CRED_LENGTH - 2, "https://beta.2ch.hk/moder/posts/%s?json=1", board_name)) {
            debug(1, "Cannot assemble URL https://beta.2ch.hk/moder/posts/%s?json=1", board_name);
            goto cleanup;
        }
    } else {
        if (!snprintf(url, MAX_CRED_LENGTH - 2, "https://2ch.hk/%s/catalog.json", board_name)) {
            debug(1, "Cannot assemble URL https://2ch.hk/%s/catalog.json", board_name);
            goto cleanup;
        }
    }

    bool res = submit_curl_task(url, cookie, &s, NULL);
    if (res) {
        cJSON* parse_result = NULL;
        if ((parse_result = cJSON_Parse(s.ptr)) == NULL) {

            const char* cjson_err = cJSON_GetErrorPtr();
            if (cjson_err) {
                debug(1, "cJSON err: %s", cjson_err);
            }
            cJSON_Delete(parse_result); // cjson checks for nullptr here
            goto cleanup;
        }

        if (as_moder) {
            dvach_populate_board_as_moder(board_name, result, parse_result);
        } else {
            dvach_populate_board(result, parse_result);
        }

        cJSON_Delete(parse_result); // cjson checks for nullptr here
    } else {
        debug(1, "submit_curl_task failed: %u ", res);
        goto cleanup;
    }

    safe_free((void**)&cookie);
    safe_free((void**)&url);
    safe_free((void**)&s.ptr);
    return result; // to be freed by caller

cleanup:
    if (result) {
        safe_free((void**)&result);
    }
    safe_free((void**)&cookie);
    safe_free((void**)&url);
    safe_free((void**)&s.ptr);
    return NULL;
}

// TODO
//post_t* fetch_all_posts_from_board(session_t* session, board_t* board) {}
//
//thread_t* fetch_thread_from_board(session_t* session, board_t* board) {}
//
//post_t* fetch_post_from_thread(session_t* session, thread_t* thread) {}
//
//file_t* fetch_file_from_post(session_t* session, post_t* post) {}

void remove_post(GtkWidget* widget, gpointer data)
{
    assert(data);
    struct g_callback_task* task = data;
}

void add_local_ban(GtkWidget* widget, gpointer data)
{
    assert(data);
    struct g_callback_task* task = data;
}

void whois(GtkWidget* widget, gpointer data)
{
    assert(data);
    struct g_callback_task* task = data;

    assert(task->what);
    const char* ip = task->what;

    debug(3, "Performing whois for %s\n", ip);

    char* url = malloc_memset(MAX_CRED_LENGTH);

    struct curl_string s = { .len = 0 };
    s.ptr = malloc_memset(s.len + 1);
    s.ptr[0] = '\0';

    if (!snprintf(url, MAX_CRED_LENGTH - 2, "http://ipwhois.app/line/%s", ip)) {
        debug(1, "Cannot assemble URL http://ipwhois.app/line/%s", ip);
        safe_free((void**)&url);
        safe_free((void**)&s.ptr);
        return;
    }

    bool res = submit_curl_task(url, "no cookie", &s, NULL);
    if (res) {
        task->caller_widget = widget;
        task->result = s.ptr; // to be freed by the caller
    } else {
        debug(1, "submit_curl_task failed: %u ", res);
    }

    safe_free((void**)&url);
}

void filter_by_ip_per_board(GtkWidget* widget, gpointer data)
{
    assert(data);
    struct g_callback_task* task = data;
}
