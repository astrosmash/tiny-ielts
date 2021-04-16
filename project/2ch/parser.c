// Functions to populate allocated data structures
// Populates allocated session struct with moder info and session cookie.
static void dvach_populate_session(session_t* session, cJSON* parse_result)
{
    // All nullptr checks have been performed by caller
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
static void dvach_populate_board(board_t* board, cJSON* parse_result)
{
    // All nullptr checks have been performed by caller
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
