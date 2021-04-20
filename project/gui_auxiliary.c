GuiRuntimeConfig* get_gui_runtime_config(bool need_to_allocate)
{
    static GuiRuntimeConfig* my_app_config = NULL;

    if (need_to_allocate) {
        my_app_config = malloc_memset(sizeof(GuiRuntimeConfig));
    }
    assert(my_app_config);
    debug(3, "Returning %p, allocated = %u\n", (void*)my_app_config, need_to_allocate);
    return my_app_config;
}

session_t* get_session(bool need_to_allocate)
{
    static session_t* session = NULL;

    if (need_to_allocate) {
        session = malloc_memset(sizeof(session_t));
    }
    assert(session);
    debug(3, "Returning %p, allocated = %u\n", (void*)session, need_to_allocate);
    return session;
}

session_creds_t* get_session_creds(bool need_to_allocate)
{
    static session_creds_t* creds = NULL;

    if (need_to_allocate) {
        creds = malloc_memset(sizeof(session_creds_t));
    }
    assert(creds);
    debug(3, "Returning %p, allocated = %u\n", (void*)creds, need_to_allocate);
    return creds;
}

gboolean update_gui(gpointer data)
{
    struct g_callback_task* task = data;
    GtkWidget* parent = task->caller_widget;
    _Gui_DrawPopupDialog(parent, task->result);
    gtk_widget_queue_draw(parent);
    safe_free(&task->result);
    return FALSE; //  If the function returns FALSE it is automatically removed from the list of event sources and will not be called again.
}

void* task_monitor(void* runtime_config)
{
    //    assert(runtime_config);
    //    GuiRuntimeConfig* config = runtime_config;

    struct timespec iter_retry = { .tv_sec = 1 };
    struct g_callback_task* task = malloc_memset(sizeof(struct g_callback_task));

    while (true) {
        for (size_t task_type = REMOVE_POST; task_type <= FILTER_BY_IP_PER_BOARD; ++task_type) {
            if (((task = task_manager(LOOK_FOR_TASKS, NULL, task_type))) != NULL) { // We have a result, let's draw it
                debug(1, "Got task result to draw: %s", task->result);
                g_idle_add(update_gui, task);
            } else {
                debug(5, "Waiting for result of tasks type %zu...", task_type);
            }
        }
        nanosleep(&iter_retry, NULL);
    }
    return NULL;
}

void* worker_func(void* data)
{
    assert(data);
    GuiRuntimeConfig* g_config = data;
    assert(g_config->window);

    debug(3, "passing board %s (cookie %s)\n", g_config->WorkerData.board, g_config->WorkerData.session->cookie);

    bool as_moder = true;
    board_as_moder_t* board = fetch_board_info(g_config->WorkerData.session, g_config->WorkerData.board, as_moder);
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

            struct g_callback_task* remove_post_task = create_new_task(REMOVE_POST, board->post);
            g_signal_connect(G_OBJECT(submenu_option_remove_post), "activate",
                G_CALLBACK(remove_post), remove_post_task);

            struct g_callback_task* add_local_ban_task = create_new_task(ADD_LOCAL_BAN, board->post);
            g_signal_connect(G_OBJECT(submenu_option_ban), "activate",
                G_CALLBACK(add_local_ban), add_local_ban_task);

            char* inserted_ip = malloc_memset(strlen(ip) + 1); // To be freed
            strncpy(inserted_ip, ip, strlen(ip));
            struct g_callback_task* whois_post_task = create_new_task(WHOIS_POST, inserted_ip);
            debug(3, "Adding IP view %s for %zu\n", whois_post_task->what, i);
            g_signal_connect(G_OBJECT(submenu_option_whois), "activate",
                G_CALLBACK(whois), whois_post_task);

            struct g_callback_task* filter_by_ip_per_board_task = create_new_task(FILTER_BY_IP_PER_BOARD, board->post);
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

static void _Gui_RunBoardTopFetchThread(GtkWidget* widget, gpointer data)
{
    assert(data);
    GuiRuntimeConfig* g_config = data;
    assert(g_config->my_gui);

    const gchar* board_name = gtk_widget_get_name(GTK_WIDGET(widget));
    memset(&g_config->WorkerData.board, 0, MAX_BOARD_NAME_LENGTH);
    strncpy(g_config->WorkerData.board, board_name, strlen(board_name));
    debug(3, "Fetching top for %s with cookie %s\n", g_config->WorkerData.board, g_config->WorkerData.session->cookie);

    assert(g_config->window);
    _Gui_CleanMainChildren(g_config->window);

    Thread* my_thread = NULL;
    if (((my_thread = Thread_Init(&worker_func, g_config)) == NULL)) {
        debug(1, "Cannot launch worker_func! %s\n", Gui_GetName(g_config->my_gui));
        return;
    }

    g_config->board_top_fetch_thread = my_thread;

    // Immediately join it
    char* thread_result = NULL;
    if (Thread_Join(g_config->board_top_fetch_thread, &thread_result) == 0) {
        debug(3, "Thread %s joined ok, result %s\n", Gui_GetName(g_config->my_gui), thread_result);
    } else {
        debug(1, "Failed to join thread %s\n", Gui_GetName(g_config->my_gui));
    }

    // Free an object allocated in the thread_func
    safe_free((void**)&thread_result);
}

// -------------------------------------------------- Drawings

static void _Gui_CleanMainChildren(GtkWidget* widget)
{
    assert(widget);

    // Remove all widgets that are present on the main window
    if (GTK_IS_CONTAINER(widget)) {
        GList* children = gtk_container_get_children(GTK_CONTAINER(widget));
        for (const GList* iter = children; iter != NULL; iter = g_list_next(iter)) {
            debug(3, "Clearing child at %p\n", iter->data);
            gtk_widget_destroy(GTK_WIDGET(iter->data));
        }
    }
}

static void _Gui_DrawPopupMenu(GtkWidget* widget, GdkEvent* event)
{
    assert(widget);
    assert(event);

    if (event->type == GDK_BUTTON_PRESS) {
        GdkEventButton* bevent = (GdkEventButton*)event;
        if (bevent->button == 3) { // Right mouse click
            gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL,
                bevent->button, bevent->time);
        }
    }
}

static void _Gui_DrawPopupDialog(GtkWidget* parent, void* what)
{
    assert(parent);
    assert(what);

    GtkWidget *dialog = NULL, *content_area = NULL, *label = NULL;
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT; // Create the widgets

    dialog = gtk_dialog_new_with_buttons("Result",
        GTK_WINDOW(parent),
        flags,
        ("_OK"),
        GTK_RESPONSE_NONE,
        NULL);
    assert(dialog);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    assert(content_area);

    // Ensure that the dialog box is destroyed when the user responds
    g_signal_connect_swapped(dialog,
        "response",
        G_CALLBACK(gtk_widget_destroy),
        dialog);

    label = gtk_label_new(what);
    assert(label);

    // Add the label, and show everything we’ve added
    gtk_container_add(GTK_CONTAINER(content_area), label);
    gtk_widget_show_all(dialog);
}

static bool _Gui_DrawMainScreen(GuiRuntimeConfig* my_app_config)
{
    assert(my_app_config->window);
    _Gui_CleanMainChildren(my_app_config->window);

    GtkWidget *box = NULL, *button = NULL, *grid = NULL, *label = NULL;

    // Main widget
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    assert(box);

    gtk_container_set_border_width(GTK_CONTAINER(box), 5);
    gtk_container_add(GTK_CONTAINER(my_app_config->window), box);
    gtk_widget_set_name(box, "main_box");

    // Grid
    grid = gtk_grid_new();
    assert(grid);
    gtk_container_add(GTK_CONTAINER(box), grid);
    gtk_widget_set_name(grid, "main_grid");

    label = gtk_label_new("Please select a board to fetch threads from.");
    assert(label);

    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
    gtk_label_set_line_wrap(GTK_LABEL(label), FALSE);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_widget_set_name(label, "select_thread_label");

    session_t* session = my_app_config->WorkerData.session;

    if (!strlen(session->cookie)) {
        // File is present - read it
        // Populate cookie and boards
        if (!populate_session_from_file(session)) {
            return false;
        }
    }

    for (size_t i = 0; i < sizeof(session->moder.boards) / sizeof(*session->moder.boards); ++i) {
        char* board_name = (char*)session->moder.boards + (MAX_BOARD_NAME_LENGTH * i);
        if (strlen(board_name)) {
            debug(3, "Adding button for (%zu) %s", i, board_name);
            button = gtk_button_new_with_label(board_name);
            assert(button);

            // Fetch posts
            g_signal_connect(button, "clicked", G_CALLBACK(_Gui_RunBoardTopFetchThread), my_app_config);
            gtk_grid_attach(GTK_GRID(grid), button, 0, i + 1, 1, 1);
            gtk_widget_set_name(button, board_name);
        }
    }
    return true;
}

static void _Gui_DrawLoginInvitationScreen(GuiRuntimeConfig* my_app_config)
{
    assert(my_app_config->window);
    GtkWidget *box = NULL, *button = NULL, *grid = NULL, *label = NULL;

    // Main widget
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    assert(box);
    gtk_container_set_border_width(GTK_CONTAINER(box), 5);
    gtk_container_add(GTK_CONTAINER(my_app_config->window), box);
    gtk_widget_set_name(box, "main_box");

    // Grid
    grid = gtk_grid_new();
    assert(grid);
    gtk_container_add(GTK_CONTAINER(box), grid);
    gtk_widget_set_name(grid, "main_grid");

    label = gtk_label_new("No local account found. Do you want to authenticate?");
    assert(label);
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
    gtk_label_set_line_wrap(GTK_LABEL(label), FALSE);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_widget_set_name(label, "no_account_warning");

    button = gtk_button_new_with_label("Yes");
    assert(button);
    g_signal_connect(button, "clicked", G_CALLBACK(_Gui_DrawAuthScreen), box);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 1, 1, 1);
    gtk_widget_set_name(button, "yes_button");

    button = gtk_button_new_with_label("Quit");
    assert(button);
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(_Gui_Exit), my_app_config);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 1, 1);
    gtk_widget_set_name(button, "quit_button");
}

static void _Gui_DrawAuthScreen(GtkWidget* widget, gpointer data)
{
    assert(data);
    GtkWidget *box = data, *grid = NULL, *label = NULL, *username_entry = NULL, *password_entry = NULL;

    // For the program lifetime - we do not want to add form on each click
    static bool pressed = false;

    if (!pressed) {
        // Grid
        grid = gtk_grid_new();
        assert(grid);
        gtk_container_add(GTK_CONTAINER(box), grid);
        gtk_widget_set_name(grid, "main_grid");

        label = gtk_label_new("Username");
        assert(label);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
        gtk_label_set_line_wrap(GTK_LABEL(label), FALSE);
        gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 1, 1);
        gtk_widget_set_name(label, "username_label");

        username_entry = gtk_entry_new();
        assert(username_entry);
        gtk_grid_attach(GTK_GRID(grid), username_entry, 0, 4, 1, 1);
        gtk_widget_set_name(username_entry, "username_entry");

        label = gtk_label_new("Password");
        assert(label);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
        gtk_label_set_line_wrap(GTK_LABEL(label), FALSE);
        gtk_grid_attach(GTK_GRID(grid), label, 0, 5, 1, 1);
        gtk_widget_set_name(label, "password_label");

        password_entry = gtk_entry_new();
        assert(password_entry);
        gtk_grid_attach(GTK_GRID(grid), password_entry, 0, 6, 1, 1);
        gtk_widget_set_name(password_entry, "password_entry");

        struct GtkEntries* entries = malloc_memset(sizeof(struct GtkEntries)); // to be freed by _Gui_GetText
        entries->entry[0] = username_entry;
        entries->entry[1] = password_entry;

        g_signal_connect(GTK_ENTRY(username_entry), "activate", G_CALLBACK(_Gui_GetText), entries);
        g_signal_connect(GTK_ENTRY(password_entry), "activate", G_CALLBACK(_Gui_GetText), entries);

        pressed = true;

        // redraw
        gtk_widget_show_all(box);
    }
}
