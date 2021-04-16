// Uses given credentials to login and obtain mod info to be later re-used.
extern ssize_t session_init(session_creds_t* creds, session_t* session)
{
    assert(creds);
    assert(session);

    struct curl_string s = { .len = 0 };
    s.ptr = malloc_memset(s.len + 1);
    s.ptr[0] = '\0';

    CURL* curl = dvach_curl_init(&s, NULL);

    char* postfields = malloc_memset(MAX_CRED_LENGTH * 5);
    if (!snprintf(postfields, (MAX_CRED_LENGTH * 5) - 2, "use_cookie=1&nick=%s&password=%s", creds->username, creds->password)) {
        debug(1, "Unable to populate postfields with credentials %s %s\n", creds->username, creds->password);
        goto cleanup;
    }

    debug(3, "Populated postfields %s\n", postfields);
    curl_easy_setopt(curl, CURLOPT_URL, "https://beta.2ch.hk/moder/login?json=1");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        debug(1, "Got curl_easy_perform err %s\n", curl_easy_strerror(res));
        goto cleanup;
    }

    size_t curl_response_code = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &curl_response_code);
    if (res != CURLE_OK) {
        debug(1, "Got curl_easy_getinfo err %s\n", curl_easy_strerror(res));
        goto cleanup;
    }

    if (curl_response_code == 200) {
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
        dvach_populate_session(session, parse_result);

        cJSON_Delete(parse_result); // cjson checks for nullptr here
    } else {
        debug(1, "CURL got http error code %zu result %s\n", curl_response_code, s.ptr);
        goto cleanup;
    }

    free(postfields);
    free(s.ptr);
    curl_easy_cleanup(curl);
    return EXIT_SUCCESS;

cleanup:
    free(postfields);
    free(s.ptr);
    curl_easy_cleanup(curl);
    return EXIT_FAILURE;
}


// Populates board info from catalog.
// Using mod cookie is optional.

extern board_t* fetch_board_info(session_t* session, const char* board_name)
{
    assert(session);
    assert(board_name);

    board_t* board = malloc_memset(sizeof(board_t));
    char* cookie = malloc_memset(MAX_CRED_LENGTH);
    char* url = malloc_memset(MAX_CRED_LENGTH);

    struct curl_string s = { .len = 0 };
    s.ptr = malloc_memset(s.len + 1);
    s.ptr[0] = '\0';

    CURL* curl = NULL;
    
    if (!snprintf(cookie, MAX_CRED_LENGTH - 2, "Cookie: %s", session->cookie)) {
        debug(1, "Cannot assemble Cookie header %s", session->cookie);
        goto cleanup;
    }

    if (!snprintf(url, MAX_CRED_LENGTH - 2, "https://2ch.hk/%s/catalog.json", board_name)) {
        debug(1, "Cannot assemble URL https://2ch.hk/%s/catalog.json", board_name);
        goto cleanup;
    }

    curl = dvach_curl_init(&s, cookie);
    curl_easy_setopt(curl, CURLOPT_URL, url);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        debug(1, "Got curl_easy_perform err %s\n", curl_easy_strerror(res));
        goto cleanup;
    }

    size_t curl_esponse_code = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &curl_esponse_code);
    if (res != CURLE_OK) {
        debug(1, "Got curl_easy_getinfo err %s\n", curl_easy_strerror(res));
        goto cleanup;
    }

    if (curl_esponse_code == 200) {
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

        dvach_populate_board(board, parse_result);

        cJSON_Delete(parse_result); // cjson checks for nullptr here
    } else {
        debug(1, "CURL got HTTP error code %zu result %s\n", curl_esponse_code, s.ptr);
        goto cleanup;
    }

    free(cookie);
    free(url);
    free(s.ptr);
    curl_easy_cleanup(curl);
    return board; // to be freed by caller

cleanup:
    free(board);
    free(cookie);
    free(url);
    free(s.ptr);
    if (curl) curl_easy_cleanup(curl);
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
