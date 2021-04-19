// Gui ctor
#define Gui_Init (*Gui_Construct)

// Global state vars
extern GuiRuntimeConfig* get_gui_runtime_config(bool need_to_allocate)
{
    static GuiRuntimeConfig* my_app_config = NULL;

    if (need_to_allocate) {
        my_app_config = malloc_memset(sizeof(GuiRuntimeConfig));
    }
    assert(my_app_config);
    debug(3, "Returning %p, allocated = %u\n", (void*)my_app_config, need_to_allocate);
    return my_app_config;
}

extern session_t* get_session(bool need_to_allocate)
{
    static session_t* session = NULL;

    if (need_to_allocate) {
        session = malloc_memset(sizeof(session_t));
    }
    assert(session);
    debug(3, "Returning %p, allocated = %u\n", (void*)session, need_to_allocate);
    return session;
}

extern session_creds_t* get_session_creds(bool need_to_allocate)
{
    static session_creds_t* creds = NULL;

    if (need_to_allocate) {
        creds = malloc_memset(sizeof(session_creds_t));
    }
    assert(creds);
    debug(3, "Returning %p, allocated = %u\n", (void*)creds, need_to_allocate);
    return creds;
}

Gui* Gui_Construct(void)
{
    Gui* g = malloc_memset(sizeof(Gui));

    _Gui_SetName(g, "2ch_worker_gui");
    debug(3, "Allocated new object on %p\n", (void*)g);

    GuiRuntimeConfig* my_app_config = get_gui_runtime_config(true);
    session_t* session = get_session(true);
    session_creds_t* creds = get_session_creds(true);

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
        fprintf(stderr, "Cannot launch thread to monitor task result! \n");
    } else {
        fprintf(stderr, "CAN launch thread to monitor task result! \n");
    }
    my_app_config->monitor_thread = my_thread;

    // Just check if a file exists without removing/creating it.
    // Necessary checks will be performed later
    if (creds_file_path(false, false) && _Gui_DrawMainScreen(my_app_config)) {
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

// --------------------------------------------------

static void* thread_func(void* data)
{
    assert(data);
    GuiRuntimeConfig* g_config = data;
    assert(g_config->window);

    debug(3, "thread_func launched ok! %s\n", Gui_GetName(g_config->my_gui));
    debug(3, "passing board %s (cookie %s)\n", g_config->WorkerData.board, g_config->WorkerData.session->cookie);

    board_as_moder_t* board = fetch_board_info_as_moder(g_config->WorkerData.session, g_config->WorkerData.board);
    assert(board); // Will be freed by the caller (_Gui_RunBoardTopFetchThread)

    // Start drawing result
    GtkWidget *grid = NULL, *vbox = NULL, *scroll = NULL;

    scroll = gtk_scrolled_window_new(NULL, NULL);
    assert(scroll);

    gtk_container_add(GTK_CONTAINER(g_config->window), scroll);
    gtk_widget_set_name(scroll, "main_scroll");
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC,
        GTK_POLICY_AUTOMATIC);

    grid = gtk_grid_new();
    assert(grid);

    gtk_widget_set_name(grid, "main_grid");
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 15);

    gtk_container_add(GTK_CONTAINER(scroll), grid);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    assert(vbox);

    for (size_t i = 0; i < sizeof(board->post) / sizeof(*board->post); ++i) {
        // Start drawing posts
        bool banned = board->post[i].banned;
        char* country = board->post[i].country;
        char* ip = board->post[i].ip;
        char* post_comment = board->post[i].comment;
        size_t post_num = board->post[i].num;
        //        size_t thread_posts_count = board->post[i].posts_count;

        for (size_t j = 0; board->post[i].country[j]; ++j) {
            board->post[i].country[j] = tolower(board->post[i].country[j]);
        }

        if (strlen(post_comment)) {
            debug(3, "Adding view for (%zu) %s/%zu\n", i, post_comment, post_num);

            // Add inner horizontal box with info per post
            GtkWidget* inner_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
            assert(inner_hbox);
            gtk_container_add(GTK_CONTAINER(vbox), inner_hbox);

            // Country flag
            GtkWidget* eventbox_flag = gtk_event_box_new();
            assert(eventbox_flag);
            gtk_box_pack_start(GTK_BOX(inner_hbox), eventbox_flag, FALSE, FALSE, 0);

            char* const image_path = malloc_memset(MAX_ARBITRARY_CHAR_LENGTH);
            assert(image_path);
            snprintf(image_path, MAX_ARBITRARY_CHAR_LENGTH - 1, "project/data/country-flags/png100px/%s.png", country);
            debug(3, "Adding flag %s for %s\n", image_path, country);

            GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file_at_size(image_path, 16, 16, NULL);
            GtkWidget* image = gtk_image_new_from_pixbuf(pixbuf);
            g_object_unref(pixbuf);
            gtk_widget_set_halign(image, GTK_ALIGN_START);
            gtk_container_add(GTK_CONTAINER(eventbox_flag), image);

            // Event box to capture clicks per post
            GtkWidget* eventbox_post = gtk_event_box_new();
            assert(eventbox_post);
            gtk_box_pack_start(GTK_BOX(inner_hbox), eventbox_post, FALSE, FALSE, 0);

            // The post
            char* const post = malloc_memset(MAX_ARBITRARY_CHAR_LENGTH);
            assert(post);
            if (!banned) {
                snprintf(post, MAX_ARBITRARY_CHAR_LENGTH - 1, "<span color='#FFCCCC'>(Banned)</span> <span color='#E0E0E0'> <i>%s</i> <b>#%zu</b> %s </span>", ip, post_num, post_comment);
            } else {
                snprintf(post, MAX_ARBITRARY_CHAR_LENGTH - 1, "<i>%s</i> <b>#%zu</b> %s", ip, post_num, post_comment);
            }

            GtkWidget* post_label = gtk_label_new(NULL);
            assert(post_label);

            gtk_label_set_justify(GTK_LABEL(post_label), GTK_JUSTIFY_CENTER);
            gtk_label_set_line_wrap(GTK_LABEL(post_label), TRUE);
            gtk_widget_set_halign(post_label, GTK_ALIGN_START);
            gtk_label_set_markup(GTK_LABEL(post_label), post);
            gtk_container_add(GTK_CONTAINER(eventbox_post), post_label);

            // Finally, separator, followed by next horizontal box
            GtkWidget* separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
            assert(separator);
            gtk_container_add(GTK_CONTAINER(vbox), separator);

            // Add context menus for posts
            GtkWidget* parent_menu = gtk_menu_new();
            assert(parent_menu);

            GtkWidget* submenu_option_remove_post = gtk_menu_item_new_with_label("Удалить пост");
            assert(submenu_option_remove_post);
            gtk_widget_show(submenu_option_remove_post);
            gtk_menu_shell_append(GTK_MENU_SHELL(parent_menu), submenu_option_remove_post);

            GtkWidget* submenu_option_ban = gtk_menu_item_new_with_label("Забанить");
            assert(submenu_option_ban);
            gtk_widget_show(submenu_option_ban);
            gtk_menu_shell_append(GTK_MENU_SHELL(parent_menu), submenu_option_ban);

            GtkWidget* submenu_option_whois = gtk_menu_item_new_with_label("Whois");
            assert(submenu_option_whois);
            gtk_widget_show(submenu_option_whois);
            gtk_menu_shell_append(GTK_MENU_SHELL(parent_menu), submenu_option_whois);

            GtkWidget* submenu_option_filter_author = gtk_menu_item_new_with_label("Все с этого IP");
            assert(submenu_option_filter_author);
            gtk_widget_show(submenu_option_filter_author);
            gtk_menu_shell_append(GTK_MENU_SHELL(parent_menu), submenu_option_filter_author);

            struct g_callback_task* remove_post_task = get_task(REMOVE_POST, board->post);
            g_signal_connect(G_OBJECT(submenu_option_remove_post), "activate",
                G_CALLBACK(remove_post), remove_post_task);

            struct g_callback_task* add_local_ban_task = get_task(ADD_LOCAL_BAN, board->post);
            g_signal_connect(G_OBJECT(submenu_option_ban), "activate",
                G_CALLBACK(add_local_ban), add_local_ban_task);

            char* inserted_ip = malloc_memset(strlen(ip) + 1); // To be freed
            strncpy(inserted_ip, ip, strlen(ip));
            struct g_callback_task* whois_post_task = get_task(WHOIS_POST, inserted_ip);
            debug(3, "adding IP view %s for %zu\n", whois_post_task->what, i);
            g_signal_connect(G_OBJECT(submenu_option_whois), "activate",
                G_CALLBACK(whois), whois_post_task);

            struct g_callback_task* filter_by_ip_per_board_task = get_task(FILTER_BY_IP_PER_BOARD, board->post);
            g_signal_connect(G_OBJECT(submenu_option_filter_author), "activate",
                G_CALLBACK(filter_by_ip_per_board), filter_by_ip_per_board_task);

            //            g_signal_connect_swapped(G_OBJECT(submenu_option_remove_post), "activate",
            //                G_CALLBACK(gtk_window_iconify), GTK_WINDOW(my_app_config->window));

            //            g_signal_connect(G_OBJECT(my_app_config->window), "destroy",
            //                G_CALLBACK(gtk_main_quit), NULL);

            g_signal_connect_swapped(G_OBJECT(eventbox_post), "button-press-event",
                G_CALLBACK(_Gui_DrawPopupMenu), parent_menu);

            ////            gtk_widget_set_size_request (label, 110, 20);
            //            gtk_widget_set_events (event_box, GDK_BUTTON_PRESS_MASK);
            //            g_signal_connect (event_box, "button_press_event",
            //                              G_CALLBACK (_Gui_DrawComboBox), label);
            //
            //            gtk_widget_realize (event_box);

            //            gtk_widget_set_name(label, thread_subject);

            //            button = gtk_button_new_with_label(country);
            //            assert(button);
            //
            //            g_signal_connect(button, "clicked", G_CALLBACK(_Gui_Exit), NULL);
            //            gtk_container_add(GTK_CONTAINER(sbox), button);
            //            gtk_widget_set_name(button, "join_thread_test_button");
            safe_free((void**)&post);
            safe_free((void**)&image_path);
        }
    }

    gtk_grid_attach(GTK_GRID(grid), vbox, 0, 1, 1, 1);
    gtk_widget_show_all(g_config->window);

    return board; // to be freed by the caller
}

static void _Gui_GetText(GtkEntry* entry, gpointer data)
{
    assert(entry);
    assert(data);
    struct GtkEntries* entries = data;

    session_creds_t* creds = get_session_creds(false);
    for (size_t i = 0; i < 2; i++) {
        assert(entries->entry[i]);

        const char* text = gtk_entry_get_text(GTK_ENTRY(entries->entry[i]));
        if (text) {
            const char* type = gtk_widget_get_name(GTK_WIDGET(entries->entry[i]));
            assert(type);

            size_t text_len = strlen(text);
            assert(text_len < MAX_CRED_LENGTH);

            if (text_len) {
                debug(3, "Read %s %s\n", type, text);
                if (strcmp(type, "username_entry") == 0) {
                    debug(3, "Read username %s\n", text);
                    if (strlen(creds->username)) {
                        memset(creds->username, 0, MAX_CRED_LENGTH);
                    }
                    strncpy(creds->username, text, text_len);

                } else if (strcmp(type, "password_entry") == 0) {
                    debug(3, "Read password %s\n", text);
                    if (strlen(creds->password)) {
                        memset(creds->password, 0, MAX_CRED_LENGTH);
                    }
                    strncpy(creds->password, text, text_len);
                } else {
                    debug(1, "text_type unknown %s, doing nothing\n", type);
                }
            }
        }
    }

    //        if (!creds)
    //            creds = malloc_memset(sizeof(session_creds_t));
    session_t* session = get_session(false);
    GuiRuntimeConfig* my_app_config = get_gui_runtime_config(false);

    if (strlen(session->cookie) == 0) {
        if (strlen(creds->username) && strlen(creds->password)) {
            debug(5, "Creds length %zu %zu\n", strlen(creds->username), strlen(creds->password));

            if (!populate_file_from_session(creds, session)) {
                return;
            }

            if (!_Gui_DrawMainScreen(my_app_config)) {
                return;
            }

            gtk_widget_show_all(my_app_config->window);
            safe_free((void**)&entries);
        }
    } else {
        debug(4, "Will not trigger session_init - have session present cookie %s user %s pass %s. Just re-launch an app to relogin\n", session->cookie, session->creds->username, session->creds->password);
    }
}
