// Gui ctor
#define Gui_Init (*Gui_Construct)

Gui* Gui_Construct(void)
{
    Gui* g = malloc_memset(sizeof(Gui));

    debug(3, "Allocated new object on %p\n", (void*)g);
    _Gui_SetName(g, "2ch_worker_gui");

    GuiRuntimeConfig* my_app_config = get_gui_runtime_config(true);
    session_t* session = get_session(true);
    session_creds_t* creds = get_session_creds(true);
    spreadsheet_t* spreadsheet = get_spreadsheet(true);

    session->creds = creds;
    my_app_config->WorkerData.session = session;

    // Start initialization
    GtkWidget* main_window = NULL;

    // Main window
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    assert(main_window);
    gtk_widget_set_name(main_window, "main_window");

    gtk_window_set_default_size(GTK_WINDOW(main_window), 1600, 800);
    gtk_window_set_title(GTK_WINDOW(main_window), "2ch-mod");

    gtk_container_set_border_width(GTK_CONTAINER(main_window), 15);
    g_signal_connect_swapped(main_window, "delete_event", G_CALLBACK(_Gui_Exit), my_app_config);

    gtk_window_present(GTK_WINDOW(main_window));

    my_app_config->my_gui = g;
    my_app_config->window = main_window;

    // Monitor results of tasks in a separate thread
    Thread* my_thread = NULL;
    if (((my_thread = Thread_Init(&task_monitor, my_app_config)) == NULL)) {
        return NULL;
    }

    my_app_config->monitor_thread = my_thread;

    // Just check if a file exists without removing/creating it.
    // Necessary checks will be performed later
    if (config_file_path(NEED_TO_CHECK) && _Gui_DrawMainScreen(my_app_config)) {
        // Session populated
    } else {
        _Gui_DrawLoginInvitationScreen(my_app_config);
    }

    gtk_widget_show_all(my_app_config->window);

    return g;
}

void Gui_Destruct(Gui** g)
{
    assert(*g);
    GuiRuntimeConfig* my_app_config = get_gui_runtime_config(false);
    if (my_app_config->board_top_fetch_thread) {
        // should be already joined by _Gui_RunBoardTopFetchThread
        Thread_Destruct(&my_app_config->board_top_fetch_thread);
    }
    if (my_app_config->monitor_thread) {
        // This is an infinite loop thread, we should send a signal to it, but let's just cancel for now...
        pthread_cancel(Thread_GetId(my_app_config->monitor_thread));
        //        if (Thread_Join(my_app_config->monitor_thread, &thread_result) == 0) {
        //            debug(3, "Thread to monitor task result joined ok, result %s\n", thread_result);
        //        } else {
        //            fprintf(stderr, "Cannot join thread to monitor task result! \n");
        //        }
        Thread_Destruct(&my_app_config->monitor_thread);
    }
    //        safe_free(&my_app_config); // bus error?

    session_t* session = get_session(false);
    if (session)
        safe_free((void**)&session);

    session_creds_t* creds = get_session_creds(false);
    if (creds)
        safe_free((void**)&creds);

    debug(3, "Freed object on %p\n", (void*)*g);
    safe_free((void**)g);

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

static void _Gui_GetAuthText(GtkEntry* entry, gpointer data)
{
    assert(entry);
    assert(data);
    struct GtkEntries* entries = data;

    spreadsheet_t* spreadsheet = get_spreadsheet(false);

    for (size_t i = 0; i < NUMBER_OF_AUTH_FORMS; i++) {
        assert(entries->entry[i]);

        const char* text = gtk_entry_get_text(GTK_ENTRY(entries->entry[i]));
        if (text) {
            const char* type = gtk_widget_get_name(GTK_WIDGET(entries->entry[i]));
            assert(type);

            size_t text_len = strlen(text);
            assert(text_len < MAX_CRED_LENGTH);

            if (text_len) {
                if (strcmp(type, "spreadsheet_key_entry") == 0) {
                    debug(3, "Read Spreadsheet Key %s\n", text);
                    if (strlen(spreadsheet->key)) {
                        memset(spreadsheet->key, 0, MAX_CRED_LENGTH);
                    }
                    strncpy(spreadsheet->key, text, text_len);

                } else if (strcmp(type, "spreadsheet_gid_entry") == 0) {
                    debug(3, "Read Spreadsheet GID %s\n", text);
                    if (strlen(spreadsheet->gid)) {
                        memset(spreadsheet->gid, 0, MAX_CRED_LENGTH);
                    }
                    strncpy(spreadsheet->gid, text, text_len);
                } else {
                    debug(1, "text_type unknown %s, doing nothing\n", type);
                }
            }
        }
    }

    // Do not need to allocate, just retrieve
    session_t* session = get_session(false);
    GuiRuntimeConfig* my_app_config = get_gui_runtime_config(false);

    if (strlen(spreadsheet->key) && strlen(spreadsheet->gid)) {
        debug(5, "Spreadsheet Key length %zu, Spreadsheet GID length %zu\n", strlen(spreadsheet->key), strlen(spreadsheet->gid));

        if (!populate_file_from_session(spreadsheet, session)) {
            return;
        }

        if (!_Gui_DrawMainScreen(my_app_config)) {
            return;
        }

        gtk_widget_show_all(my_app_config->window);
        safe_free((void**)&entries);
    }
}
