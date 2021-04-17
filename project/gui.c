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
    creds = malloc_memset(sizeof(session_creds_t));
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
    assert(my_app_config->window);

    assert(data);
    GuiRuntimeConfig* g_config = data;

    debug(3, "thread_func launched ok! %s\n", Gui_GetName(g_config->my_gui));
    debug(3, "passing board %s (cookie %s)\n", g_config->WorkerData.board, g_config->WorkerData.session->cookie);

    board_as_moder_t* board = fetch_board_info_as_moder(g_config->WorkerData.session, g_config->WorkerData.board);
    assert(board); // Will be freed by the callee (_Gui_RunChildThread)

    // Start drawing result
    GtkWidget *grid = NULL, *label = NULL, *sbox = NULL, *scroll = NULL, *button = NULL, *event_box = NULL, *pmenu = NULL, *pmenu1 = NULL, *pmenu2 = NULL, *eventbox2 = NULL;

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
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 15);

    gtk_container_add(GTK_CONTAINER(scroll), grid);

    sbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 30);
    assert(sbox);

    for (size_t i = 0; i < sizeof(board->post) / sizeof(*board->post); ++i) {
        // Start drawing posts
        size_t post_num = board->post[i].num;
        size_t thread_posts_count = board->post[i].posts_count;
        char* post_comment = board->post[i].comment;
        char* country = board->post[i].country;
        char* ip = board->post[i].ip;

        for (size_t j = 0; board->post[i].country[j]; ++j) {
            board->post[i].country[j] = tolower(board->post[i].country[j]);
        }

        if (strlen(post_comment)) {
            debug(3, "Adding view for (%zu) %s/%zu\n", i, post_comment, post_num);

            char* const label_name = malloc_memset(MAX_ARBITRARY_CHAR_LENGTH);
            assert(label_name);

            snprintf(label_name, MAX_ARBITRARY_CHAR_LENGTH - 1, "%zu %s (%s, %s)", post_num, post_comment, country, ip);

            event_box = gtk_event_box_new();
            assert(event_box);
            gtk_container_add(GTK_CONTAINER(sbox), event_box);
            //            gtk_widget_show(event_box);

            label = gtk_label_new(label_name);
            assert(label);

            gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
            gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
            gtk_widget_set_halign(label, GTK_ALIGN_START);
            gtk_container_add(GTK_CONTAINER(event_box), label);

            eventbox2 = gtk_event_box_new();
            assert(eventbox2);
            gtk_container_add(GTK_CONTAINER(sbox), eventbox2);

            char* const image_path = malloc_memset(MAX_ARBITRARY_CHAR_LENGTH);
            assert(image_path);
            snprintf(image_path, MAX_ARBITRARY_CHAR_LENGTH - 1, "project/data/country-flags/png100px/%s.png", country);
            debug(3, "Adding flag %s for %s\n", image_path, country);

            GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file_at_size(image_path, 16, 16, NULL);
            GtkWidget* image = gtk_image_new_from_pixbuf(pixbuf);
            g_object_unref(pixbuf);
            gtk_container_add(GTK_CONTAINER(eventbox2), image);

            pmenu = gtk_menu_new();

            pmenu1 = gtk_menu_item_new_with_label("Бан нахуй");
            gtk_widget_show(pmenu1);
            gtk_menu_shell_append(GTK_MENU_SHELL(pmenu), pmenu1);

            pmenu2 = gtk_menu_item_new_with_label("Репорт цп");
            gtk_widget_show(pmenu2);
            gtk_menu_shell_append(GTK_MENU_SHELL(pmenu), pmenu2);

            g_signal_connect_swapped(G_OBJECT(pmenu1), "activate",
                G_CALLBACK(gtk_window_iconify), GTK_WINDOW(my_app_config->window));

            g_signal_connect(G_OBJECT(pmenu2), "activate",
                G_CALLBACK(gtk_main_quit), NULL);

            g_signal_connect(G_OBJECT(my_app_config->window), "destroy",
                G_CALLBACK(gtk_main_quit), NULL);

            g_signal_connect_swapped(G_OBJECT(event_box), "button-press-event",
                G_CALLBACK(_Gui_DrawPopup), pmenu);

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
            free(label_name);
            free(image_path);
        }
    }

    gtk_grid_attach(GTK_GRID(grid), sbox, 0, 1, 1, 1);
    gtk_widget_show_all(my_app_config->window);

    return board; // to be freed by the caller
}

static void _Gui_GetText(GtkEntry* entry, gpointer data)
{
    assert(entry);
    assert(data);
    struct GtkEntries* entries = data;

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
            free(entries);
        }
    } else {
        debug(4, "Will not trigger session_init - have session present cookie %s user %s pass %s. Just re-launch an app to relogin\n", session->cookie, session->creds->username, session->creds->password);
    }
}
