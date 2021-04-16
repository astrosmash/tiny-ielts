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
    char* fullpath = malloc_memset(fullpathsize);

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
    CURL* curl = curl_easy_init();
    ;
    assert(curl);

    struct curl_slist* headers = NULL;
    char* user_agent = malloc_memset(MAX_CRED_LENGTH);

    if (!snprintf(user_agent, MAX_CRED_LENGTH - 2, "User-Agent: 2ch-mod/%f", 0.1)) {
        debug(1, "Unable to populate User-Agent %s\n", user_agent);
        free(user_agent);
        return NULL;
    }

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
