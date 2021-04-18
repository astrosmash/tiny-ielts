// Authentication
extern bool populate_session_from_file(session_t* session)
{
    assert(session);

    char* fullpath = creds_file_path(false, false);
    assert(fullpath);

    debug(3, "Reading credentials from %s\n", fullpath);
    const char* opmode = "r";
    FILE* file = NULL;

    if ((file = fopen(fullpath, opmode)) == NULL) {
        debug(1, "fopen(%s) no file\n", fullpath);
        free(fullpath);
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
        fullpath = creds_file_path(false, true); // remove file
        free(buf);
        free(fullpath);
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
    free(buf);
    free(fullpath);
    return true;
}

// Refresh cookie and other contents in the file using session data
extern bool populate_file_from_session(session_creds_t* creds, session_t* session)
{
    assert(creds);
    assert(session);

    debug(3, "Triggering session_init with user %s pass %s\n", creds->username, creds->password);

    bool allowed = false;
    for (size_t i = 0; i < ARRAY_SIZE(client_whitelisted_users); i++) {
        if (strcmp(*(client_whitelisted_users + i), creds->username) == 0) {
            allowed = true;
        }
    }
    if (!allowed) {
        debug(1, "You are not whitelisted to use this client! %s", creds->username);
        return false;
    }

    ssize_t session_res = 0;
    if ((session_res = session_init(creds, session))) {
        debug(1, "Was not able to trigger session_init with user %s pass %s (%zd)\n", creds->username, creds->password, session_res);
        return false;
    }

    // No file was present and we are authenticating. Create a file
    char* fullpath = creds_file_path(true, false);
    assert(fullpath);
    debug(3, "Writing credentials to %s\n", fullpath);

    const char* mode = "w";
    FILE* file = NULL;

    if ((file = fopen(fullpath, mode)) == NULL) {
        debug(1, "fopen(%s) cannot open file\n", fullpath);
        free(fullpath);
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
            free(add);
        }
    }

    size_t ret = fwrite(content, sizeof(char), strlen(content), file);
    if (!ret) {
        debug(1, "fwrite(%s) cannot write to file\n", fullpath);
        fclose(file);
        free(content);
        free(fullpath);
        return false;
    }

    fclose(file);
    free(content);
    free(fullpath);
    return true;
}

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

    if (!snprintf(cookie, MAX_CRED_LENGTH - 2, "Cookie: moder=%s", session->cookie)) {
        debug(1, "Cannot assemble Cookie header moder=%s", session->cookie);
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
    if (curl)
        curl_easy_cleanup(curl);
    return NULL;
}

// Populates board info from mod api.
// Using mod cookie is mandatory.

extern board_as_moder_t* fetch_board_info_as_moder(session_t* session, const char* board_name)
{
    assert(session);
    assert(board_name);

    board_as_moder_t* board = malloc_memset(sizeof(board_as_moder_t));
    char* cookie = malloc_memset(MAX_CRED_LENGTH);
    char* url = malloc_memset(MAX_CRED_LENGTH);

    struct curl_string s = { .len = 0 };
    s.ptr = malloc_memset(s.len + 1);
    s.ptr[0] = '\0';

    CURL* curl = NULL;

    if (!snprintf(cookie, MAX_CRED_LENGTH - 2, "Cookie: moder=%s", session->cookie)) {
        debug(1, "Cannot assemble Cookie header moder=%s", session->cookie);
        goto cleanup;
    }

    if (!snprintf(url, MAX_CRED_LENGTH - 2, "https://beta.2ch.hk/moder/posts/%s?json=1", board_name)) {
        debug(1, "Cannot assemble URL https://beta.2ch.hk/moder/posts/%s?json=1", board_name);
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

        dvach_populate_board_as_moder(board_name, board, parse_result);

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
    if (curl)
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

extern void remove_post(GtkWidget* widget, gpointer data)
{
    assert(data);
    struct g_callback_task* task = data;
}

extern void add_local_ban(GtkWidget* widget, gpointer data)
{
    assert(data);
    struct g_callback_task* task = data;
}

extern void whois(GtkWidget* widget, gpointer data)
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

    CURL* curl = NULL;

    if (!snprintf(url, MAX_CRED_LENGTH - 2, "http://ipwhois.app/line/%s", ip)) {
        debug(1, "Cannot assemble URL http://ipwhois.app/line/%s", ip);
        free(url);
        free(s.ptr);
        return;
    }

    curl = dvach_curl_init(&s, "no cookie"); // no cookie
    curl_easy_setopt(curl, CURLOPT_URL, url);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        debug(1, "Got curl_easy_perform err %s\n", curl_easy_strerror(res));
        free(url);
        free(s.ptr);
        curl_easy_cleanup(curl);
        return;
    }

    size_t curl_esponse_code = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &curl_esponse_code);
    if (res != CURLE_OK) {
        debug(1, "Got curl_easy_getinfo err %s\n", curl_easy_strerror(res));
        free(url);
        free(s.ptr);
        curl_easy_cleanup(curl);
        return;
    }

    if (curl_esponse_code == 200) {
        debug(3, "%s CURL result %s\n", url, s.ptr);

        GtkWidget *dialog, *label, *content_area;
        GtkDialogFlags flags;

        // Create the widgets
        flags = GTK_DIALOG_DESTROY_WITH_PARENT;
        dialog = gtk_dialog_new_with_buttons("Message",
            GTK_WINDOW(widget),
            flags,
            ("_OK"),
            GTK_RESPONSE_NONE,
            NULL);
        content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        label = gtk_label_new(s.ptr);

        // Ensure that the dialog box is destroyed when the user responds
        g_signal_connect_swapped(dialog,
            "response",
            G_CALLBACK(gtk_widget_destroy),
            dialog);

        // Add the label, and show everything we’ve added
        gtk_container_add(GTK_CONTAINER(content_area), label);
        gtk_widget_show_all(dialog);
    } else {
        debug(1, "CURL got HTTP error code %zu result %s\n", curl_esponse_code, s.ptr);
        free(url);
        free(s.ptr);
        curl_easy_cleanup(curl);
        return;
    }

    free(url);
    free(s.ptr);
    curl_easy_cleanup(curl);
}

extern void filter_by_ip_per_board(GtkWidget* widget, gpointer data)
{
    assert(data);
    struct g_callback_task* task = data;
}
