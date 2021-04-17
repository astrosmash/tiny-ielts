static void _Gui_RunChildThread(GtkWidget* widget, gpointer data)
{
    assert(data);
    GuiRuntimeConfig* g_config = data;
    assert(g_config->my_gui);

    Thread* my_thread = NULL;

    const gchar* board_name = gtk_widget_get_name(GTK_WIDGET(widget));
    memset(&g_config->WorkerData.board, 0, MAX_BOARD_NAME_LENGTH);
    strncpy(g_config->WorkerData.board, board_name, strlen(board_name));
    debug(3, "Passing %s(%s)\n", g_config->WorkerData.board, g_config->WorkerData.session->cookie);

    assert(g_config->window);
    _Gui_CleanMainChildren(g_config->window);

    if (((my_thread = Thread_Init(&thread_func, g_config)) == NULL)) {
        debug(1, "Cannot launch thread_func! %s\n", Gui_GetName(g_config->my_gui));
    }

    g_config->child_thread = my_thread;

    // Immediately join it
    char* thread_result = NULL;
    if (Thread_Join(g_config->child_thread, &thread_result) == 0) {
        debug(3, "Thread %s joined ok, result %s\n", Gui_GetName(g_config->my_gui), thread_result);
    } else {
        debug(1, "Failed to join thread %s\n", Gui_GetName(g_config->my_gui));
    }

    // Free an object allocated in the thread_func
    free(thread_result);
}

// -------------------------------------------------- Drawings

static void _Gui_CleanMainChildren(GtkWidget* window)
{
    assert(window);

    // Remove all widgets that are present on the main window
    if (GTK_IS_CONTAINER(window)) {
        GList* children = gtk_container_get_children(GTK_CONTAINER(window));
        fprintf(stdout, "Checking child containers...\n");

        for (const GList* iter = children; iter != NULL; iter = g_list_next(iter)) {
            fprintf(stdout, "Found child, destroying\n");
            gtk_widget_destroy(GTK_WIDGET(iter->data));
        }
    }
}

static bool _Gui_DrawPopup(GtkWidget* widget, GdkEvent* event)
{
    assert(widget);
    assert(event);

    if (event->type == GDK_BUTTON_PRESS) {
        GdkEventButton* bevent = (GdkEventButton*)event;
        if (bevent->button == 3) { // Right mouse click
            gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL,
                bevent->button, bevent->time);
        }
        return true;
    }
    return false;
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
            g_signal_connect(button, "clicked", G_CALLBACK(_Gui_RunChildThread), my_app_config);
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
    g_signal_connect(button, "clicked", G_CALLBACK(_Gui_WantAuthenticate), box);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 1, 1, 1);
    gtk_widget_set_name(button, "yes_button");

    button = gtk_button_new_with_label("Quit");
    assert(button);
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(_Gui_Exit), my_app_config);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 1, 1);
    gtk_widget_set_name(button, "quit_button");
}

static void _Gui_WantAuthenticate(GtkWidget* widget, gpointer data)
{
    assert(data);
    GtkWidget *box = data, *grid = NULL, *label = NUL, *username_entry = NULL, *password_entry = NULL;

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
