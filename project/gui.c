// Gui ctor
#define Gui_Init (*Gui_Construct)

// Global state vars
GuiRuntimeConfig* my_app_config = NULL;
session_creds_t* creds = NULL;
session_t* session = NULL;

Gui* Gui_Construct(void)
{
    Gui* g = malloc_memset(sizeof(Gui));

    _Gui_SetName(g, "2ch_worker_gui");
    debug(3, "Allocated new object on %p\n", (void*)g);

    // Allocate global state vars
    my_app_config = malloc_memset(sizeof(GuiRuntimeConfig));
    session = malloc_memset(sizeof(session_t));

    // Start initialization
    GtkWidget* main_window = NULL;

    // Main window
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    assert(main_window);
    gtk_widget_set_name(main_window, "main_window");

    gtk_window_set_default_size(GTK_WINDOW(main_window), 1600, 800);
    gtk_window_set_title(GTK_WINDOW(main_window), "2ch-mod");

    gtk_container_set_border_width(GTK_CONTAINER(main_window), 10);
    g_signal_connect_swapped(main_window, "delete_event", G_CALLBACK(_Gui_Exit), my_app_config);

    gtk_window_present(GTK_WINDOW(main_window));

    my_app_config->my_gui = g;
    my_app_config->window = main_window;

    // Just check if a file exists without removing/creating it.
    // Necessary checks will be performed later
    if (creds_file_path(false, false) && _Gui_DrawMainScreen(my_app_config, session)) {
        // Session populated
        my_app_config->WorkerData.session = session;
    } else {
        _Gui_DrawLoginInvitationScreen(my_app_config);
    }

    gtk_widget_show_all(my_app_config->window);

    return g;
}

void Gui_Destruct(Gui** g)
{
    assert(*g);

    if (my_app_config->child_thread)
        Thread_Destruct(&my_app_config->child_thread);
    //        free(my_app_config);
    //        my_app_config = NULL; // bus error?

    if (session)
        free(session);
    session = NULL;

    if (creds)
        free(creds);
    creds = NULL;

    debug(3, "Freed object on %p\n", (void*)*g);
    free(*g);
    *g = NULL;

    gtk_main_quit();
}

// Getters

char* Gui_GetName(Gui* const g)
{
    assert(g);
    return g->name;
}

// Private methods

static void _Gui_SetName(Gui* g, char* name)
{
    assert(name);
    strncpy(g->name, name, strlen(name));
}

// Callback for exit button that calls dtor, called with swapped params

static void _Gui_Exit(gpointer data, GtkWidget* widget)
{
    assert(data);
    GuiRuntimeConfig* g_config = data;
    debug(3, "freeing %s\n", Gui_GetName(g_config->my_gui));
    Gui_Destruct(&g_config->my_gui);
}

// --------------------------------------------------

static void* thread_func(void* data)
{
    assert(data);
    GuiRuntimeConfig* g_config = data;

    debug(3, "thread_func launched ok! %s\n", Gui_GetName(g_config->my_gui));
    debug(3, "passing board %s (cookie %s)\n", g_config->WorkerData.board, g_config->WorkerData.session->cookie);

    board_t* board = fetch_board_info(g_config->WorkerData.session, g_config->WorkerData.board);
    assert(board);

    assert(my_app_config->window);

    //_Gui_CleanMainChildren(); // this should happen only once!

    // Start drawing result
    GtkWidget *grid = NULL, *label = NULL, *sbox = NULL, *scroll = NULL;

    scroll = gtk_scrolled_window_new(NULL, NULL);
    assert(scroll);

    gtk_container_add(GTK_CONTAINER(my_app_config->window), scroll);
    gtk_widget_set_name(scroll, "main_scroll");
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC,
        GTK_POLICY_AUTOMATIC);

    grid = gtk_grid_new();
    assert(grid);

    gtk_widget_set_name(grid, "main_grid");
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);

    gtk_container_add(GTK_CONTAINER(scroll), grid);

    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);

    sbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    assert(sbox);

    for (size_t i = 0; i < sizeof(board->thread) / sizeof(*board->thread); ++i) {
        // Start drawing threads
        size_t thread_num = board->thread[i].num;
        //        size_t thread_posts_count = board->thread[i].posts_count;
        char* thread_subject = board->thread[i].subject;
        //        char* thread_date = board->thread[i].date;

        if (strlen(thread_subject)) {
            debug(3, "Adding view for (%zu) %s/%zu\n", i, thread_subject, thread_num);

            label = gtk_label_new(thread_subject);
            assert(label);

            //            gtk_widget_set_name(label, thread_subject);
            //            gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
            //            gtk_label_set_line_wrap(GTK_LABEL(label), FALSE);

            gtk_container_add(GTK_CONTAINER(sbox), label);
        }
    }

    gtk_grid_attach(GTK_GRID(grid), sbox, 0, 1, 1, 1);

    //    // Test button to join the thread that generated the info above
    //    GtkWidget* button = NULL;
    //    button = gtk_button_new_with_label("Join thread (test)");
    //    assert(button);
    //
    //    g_signal_connect(button, "clicked", G_CALLBACK(_Gui_JoinThread), my_app_config);
    //    gtk_container_add(GTK_CONTAINER(sbox), button);
    //    gtk_widget_set_name(button, "join_thread_test_button");

    gtk_widget_show_all(my_app_config->window);
    //    free(board);
    //    do_network(data, 0);
    return board; // to be freed
}

static void _Gui_GetText(GtkEntry* entry, gpointer data)
{
    assert(entry);
    assert(data);
    size_t text_type = *(size_t*)data;

    const char* text = NULL;
    text = gtk_entry_get_text(entry);
    assert(text);
    size_t text_len = strlen(text);

    if (text_len) {
        if (!creds)
            creds = malloc_memset(sizeof(session_creds_t));

        if (text_type == Username) {
            // Make sure no overflow occurs
            assert(text_len < MAX_CRED_LENGTH);
            debug(3, "Read username %s\n", text);
            if (strlen(creds->username)) {
                memset(creds->username, 0, MAX_CRED_LENGTH);
            }
            strncpy(creds->username, text, text_len);
        } else if (text_type == Password) {
            // Make sure no overflow occurs
            assert(text_len < MAX_CRED_LENGTH);
            debug(3, "Read password %s\n", text);
            if (strlen(creds->password)) {
                memset(creds->password, 0, MAX_CRED_LENGTH);
            }
            strncpy(creds->password, text, text_len);
        } else {
            debug(1, "text_type unknown %zu, doing nothing\n", text_type);
        }

        if (strlen(session->cookie) == 0) {
            if (strlen(creds->username) && strlen(creds->password)) {
                debug(3, "Triggering session_init with user %s pass %s\n", creds->username, creds->password);

                bool allowed = false;
                for (size_t i = 0; i < ARRAY_SIZE(client_whitelisted_users); i++) {
                    if (strcmp(*(client_whitelisted_users + i), creds->username) == 0) {
                        allowed = true;
                    }
                }
                if (!allowed) {
                    debug(1, "You are not whitelisted to use this client! %s", creds->username);
                    return;
                }

                ssize_t session_res = 0;

                if ((session_res = session_init(creds, session))) {
                    debug(1, "Was not able to trigger session_init with user %s pass %s (%zd)\n", creds->username, creds->password, session_res);
                    return;
                }
                my_app_config->WorkerData.session = session;

                // No file was present and we are authenticating. Create a file
                char* fullpath = creds_file_path(true, false);
                assert(fullpath);
                debug(3, "Writing credentials to %s\n", fullpath);

                const char* mode = "w";
                FILE* file = NULL;

                if ((file = fopen(fullpath, mode)) == NULL) {
                    debug(1, "fopen(%s) cannot open file\n", fullpath);
                    free(fullpath);
                    return;
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
                    return;
                }

                fclose(file);
                free(content);
                free(fullpath);

                if (!_Gui_DrawMainScreen(my_app_config, session))
                    return;
                // Session populated
                my_app_config->WorkerData.session = session;
                gtk_widget_show_all(my_app_config->window);
            }
        } else {
            debug(4, "Will not trigger session_init - have session present cookie %s user %s pass %s. Just re-launch an app to relogin\n", session->cookie, session->creds->username, session->creds->password);
        }
    }
}
