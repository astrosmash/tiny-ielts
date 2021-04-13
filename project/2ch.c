extern size_t check_local_account(void)
{
    size_t res = 0;
    const char *homedir = NULL;

    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    assert(homedir);
    debug("homedir is %s\n", homedir);

    const char* dvach_subdir = "/.mod2ch";
    const char* dvach_account_file = "/.creds";

    size_t fullpathsize = strlen(homedir) + strlen(dvach_subdir) + strlen(dvach_account_file) + 2;
    char *fullpath = malloc(fullpathsize);
    assert(fullpath);

    memset(fullpath, 0, fullpathsize);

    strncpy(fullpath, homedir, strlen(homedir));

    struct stat stat_buf = {0};

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
    struct stat stat_buf = {0};

    if ((stat_res = stat(check_path, &stat_buf))) {
        debug("%d\n", errno);
    }

    return stat_res;
}

static size_t curl_write_func(void *ptr, size_t size, size_t nmemb, struct curl_string *s) {
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


extern ssize_t session_init(session_creds_t* creds)
{
    assert(creds);

    char* postfields = malloc(MAX_CRED_LENGTH * 5);
    assert(postfields);

    ssize_t func_res = EXIT_SUCCESS;

    if (!snprintf(postfields, (MAX_CRED_LENGTH * 3) - 2, "use_cookie=1&nick=%s&password=%s", creds->username, creds->password)) {
        debug("cannot populate postfields with %s %s\n", creds->username, creds->password);
        func_res = EXIT_FAILURE;
        return func_res;
    }

    debug("called with %s\n", postfields);

    CURL *curl = NULL;
    CURLcode res = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    assert(curl);

    struct curl_string s = {.len = 0};
    s.ptr = malloc(s.len + 1);
    assert(s.ptr);
    s.ptr[0] = '\0';
 
    curl_easy_setopt(curl, CURLOPT_URL, "https://beta.2ch.hk/moder/login?json=1");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);

    res = curl_easy_perform(curl);

    debug("CURL result %s\n", s.ptr);

    free(s.ptr);
    s.ptr = NULL;

    if (res != CURLE_OK) {
        debug("got curl err %s\n", curl_easy_strerror(res));
        func_res = EXIT_FAILURE;

        curl_easy_cleanup(curl);

        return func_res;
    }

    curl_global_cleanup();

    return func_res;
}

