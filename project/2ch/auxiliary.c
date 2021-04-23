const char* get_homedir(void)
{
    const char* homedir = NULL;

    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    assert(homedir);
    debug(3, "Determined ${HOME} to be %s\n", homedir);

    return homedir;
}

char* config_file_path(size_t mode)
{
    const char* homedir = get_homedir();
    const char* config_subdir = "/.tiny-ielts";
    const char* config_file = "/.config";

    size_t fullpathsize = strlen(homedir) + strlen(config_subdir) + strlen(config_file) + 1;
    char* fullpath = malloc_memset(fullpathsize);

    strncpy(fullpath, homedir, strlen(homedir));
    strncat(fullpath, config_subdir, strlen(config_subdir));
    debug(3, "Determined configuration directory to be %s\n", fullpath);

    size_t res = 0;
    struct stat stat_buf = { 0 };
    if ((res = stat(fullpath, &stat_buf))) {
        debug(4, "Cannot access configuration directory %s (%s)\n", fullpath, strerror(errno));

        if (mode & NEED_TO_CREATE) {
            if ((stat_buf.st_mode & S_IFMT) != S_IFDIR) {
                debug(4, "%s is not a directory, will try to create...\n", fullpath);
            }

            if ((res = mkdir(fullpath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))) {
                debug(1, "Cannot create credentials directory %s (%s)\n", fullpath, strerror(errno));
                safe_free((void**)&fullpath);
                return NULL;
            }
        } else {
            // Cannot access a directory and need_to_create = false
            safe_free((void**)&fullpath);
            return NULL;
        }
    }

    strncat(fullpath, config_file, strlen(config_file));
    debug(3, "Determined configuration file to be %s\n", fullpath);

    if ((res = stat(fullpath, &stat_buf))) {
        debug(4, "Cannot access configuration file %s (%s)\n", fullpath, strerror(errno));

        if (mode & NEED_TO_CREATE) {
            if ((stat_buf.st_mode & S_IFMT) != S_IFREG) {
                debug(4, "%s is not a file, will try to create...\n", fullpath);
            }

            if ((res = open(fullpath, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) == -1) {
                debug(1, "Cannot create credentials file %s (%s)\n", fullpath, strerror(errno));
                safe_free((void**)&fullpath);
                return NULL;
            }
        } else {
            // Cannot access a file and need_to_create = false
            debug(1, "Cannot access a file and need_to_create = false %s\n", fullpath);
            safe_free((void**)&fullpath);
            return NULL;
        }
    }

    if (mode & NEED_TO_DELETE) {
        if (unlink(fullpath)) {
            debug(1, "Was not able to remove %s (%s)\n", fullpath, strerror(errno));
        }
    }

    // to be freed by caller.
    return fullpath;
}

void* task_manager(size_t mode, void* task, size_t type)
{
    static size_t allocated_tasks = 0;
    static struct g_callback_task* pool[MAX_NUM_OF_TASKS] = { NULL };
    struct g_callback_task* task_to_provide = NULL;

    if (mode & INSERT_TASK) { // insert mode
        struct g_callback_task* insert_task = task;
        insert_task->type = type;
        assert(allocated_tasks <= MAX_NUM_OF_TASKS);
        debug(1, "inserting task %p to pool as #%zu (type %zu)", (void*)insert_task, allocated_tasks, type);
        pool[allocated_tasks] = insert_task;
        ++allocated_tasks;
        return NULL;
    } else if (mode & LOOK_FOR_TASKS) {
        for (size_t i = 0; i <= MAX_NUM_OF_TASKS; ++i) {
            //            debug(1, "searching for tasks of type %zu, now at %zu", type, i);
            if (((task_to_provide = pool[i])) && (task_to_provide->type == type && task_to_provide->result)) { // hit
                debug(1, "found match of type #%zu at %p", task_to_provide->type, (void*)task_to_provide);
                return task_to_provide;
            }
        }
    }
    return NULL;
}

struct g_callback_task* create_new_task(size_t task_type, void* what)
{
    struct g_callback_task* remove_post_task = malloc_memset(sizeof(struct g_callback_task));
    struct g_callback_task* add_local_ban_task = malloc_memset(sizeof(struct g_callback_task));
    struct g_callback_task* whois_post_task = malloc_memset(sizeof(struct g_callback_task));
    struct g_callback_task* filter_by_ip_per_board_task = malloc_memset(sizeof(struct g_callback_task));

    switch (task_type) {
    case REMOVE_POST:
        remove_post_task->what = what;
        task_manager(INSERT_TASK, remove_post_task, task_type);
        return remove_post_task;

    case ADD_LOCAL_BAN:
        add_local_ban_task->what = what;
        task_manager(INSERT_TASK, add_local_ban_task, task_type);
        return add_local_ban_task;

    case WHOIS_POST:
        whois_post_task->what = what;
        task_manager(INSERT_TASK, whois_post_task, task_type);
        return whois_post_task;

    case FILTER_BY_IP_PER_BOARD:
        filter_by_ip_per_board_task->what = what;
        task_manager(INSERT_TASK, filter_by_ip_per_board_task, task_type);
        return filter_by_ip_per_board_task;

    default:
        debug(1, "Unknown task type %zu\n", task_type);
        return NULL;
    }
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
    assert(curl);

    struct curl_slist* headers = NULL;
    char* user_agent = malloc_memset(MAX_CRED_LENGTH);

    if (!snprintf(user_agent, MAX_CRED_LENGTH - 2, "User-Agent: 2ch-mod/%f", 0.1)) {
        debug(1, "Unable to populate User-Agent %s\n", user_agent);
        safe_free((void**)&user_agent);
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
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, s);

    safe_free((void**)&user_agent);
    return curl;
}

static bool populate_translation(char* what, translation_t* translation)
{
    assert(what);
    assert(translation);

    struct {
        char* lang_code;
        char* lang_name;
    } lang_mapping[] = {
        { "ua", "украинский" },
        { "ru", "русский" },
        { "en", "английский" }
    };

    char* strtok_saveptr = NULL;
    char* tokenized = strtok_r(what, ",", &strtok_saveptr);

    size_t processed_words = 0; // 4 in a row.
    while (tokenized != NULL) {
        size_t tok_len = strlen(tokenized);
        debug(3, "Parsing tokenized: %s (len %zu) processed_words %zu\n", tokenized, tok_len, processed_words);

        switch (processed_words) {
        case 0:
            for (size_t i = 0; i < sizeof(lang_mapping) / sizeof(*lang_mapping); ++i) {
                if (strcmp(lang_mapping[i].lang_name, tokenized) == 0) {
                    size_t len = strlen(lang_mapping[i].lang_code);
                    strncpy(translation->or_lang_code, lang_mapping[i].lang_code, len);
                    translation->or_lang_code[len] = '\0';
                    break;
                }
            }
            break;
        case 1:
            for (size_t i = 0; i < sizeof(lang_mapping) / sizeof(*lang_mapping); ++i) {
                if (strcmp(lang_mapping[i].lang_name, tokenized) == 0) {
                    size_t len = strlen(lang_mapping[i].lang_code);
                    strncpy(translation->tr_lang_code, lang_mapping[i].lang_code, len);
                    translation->tr_lang_code[len] = '\0';
                    break;
                }
            }
            break;
        case 2:
            strncpy(translation->or_word, tokenized, tok_len);
            translation->or_word[tok_len] = '\0';
            break;
        case 3:
            strncpy(translation->tr_word, tokenized, tok_len - 1); // trim last ^M from strtok
            translation->tr_word[tok_len - 1] = '\0';
            break;
        default:
            debug(3, "Unknown processed_words: %zu\n", processed_words);
            return false;
        }

        tokenized = strtok_r(NULL, ",", &strtok_saveptr);
        ++processed_words;
    }
    return true;
}

translation_response_t* parse_csv(char* csv)
{
    assert(csv);
    translation_response_t* response = malloc_memset(sizeof(translation_t));
    size_t translated_lines = 0;

    char* strtok_saveptr = NULL;
    char* line = strtok_r(csv, "\n", &strtok_saveptr);
    while (line != NULL) {
        char* newstr = malloc_memset(strlen(line) + 2);
        size_t commas = 0;
        size_t populated_chars = 0;

        for (size_t i = 0; i <= strlen(line); ++i) {
            if (*(line + i) == '"') {
                debug(3, "Skipping %c at %p \n", *(line + i), (void*)(line + i));
                continue;
            }
            if (*(line + i) == ',') {
                if (commas > 2) {
                    debug(3, "Skipping %c at %p \n", *(line + i), (void*)(line + i));
                    continue;
                }
                ++commas;
            }

            *(newstr + populated_chars) = *(line + i);
            ++populated_chars;
        }
        *(newstr + populated_chars) = '\0'; // Last char
        debug(3, "Parsing line: %s\n\n", newstr);

        assert(translated_lines <= MAX_TRANSLATIONS);
        translation_t* translation = malloc_memset(sizeof(translation_t)); // to be freed by caller
        assert(populate_translation(newstr, translation));

        response->num_of_translations = translated_lines;
        response->result[translated_lines] = translation;
        debug(3, "Populated translation #%zu(%p): %s(%s) > %s(%s)\n",
            translated_lines,
            (void*)translation,
            response->result[translated_lines]->or_word,
            response->result[translated_lines]->or_lang_code,
            response->result[translated_lines]->tr_word,
            response->result[translated_lines]->tr_lang_code);
        translated_lines++;

        safe_free((void**)&newstr);
        line = strtok_r(NULL, "\n", &strtok_saveptr);
    }
    return response;
}

static bool submit_curl_task(const char* url, const char* cookie, struct curl_string* s, void* postfields)
{
    assert(url);
    assert(s);

    CURL* curl = dvach_curl_init(s, cookie);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    if (postfields) { // POST mode
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);
    }

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        debug(1, "Got curl_easy_perform err %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return false;
    }

    size_t curl_esponse_code = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &curl_esponse_code);
    if (res != CURLE_OK) {
        debug(1, "Got curl_easy_getinfo err %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return false;
    }

    if (curl_esponse_code == 200) {
        debug(3, "%s CURL result %s\n", url, s->ptr);
    } else {
        debug(1, "CURL got HTTP error code %zu result %s\n", curl_esponse_code, s->ptr);
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_cleanup(curl);
    return true;
}
