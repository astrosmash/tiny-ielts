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
}

static bool _Gui_PopulateSessionFromFile(const char* fullpath, session_t* session)
{
    assert(fullpath);
    assert(session);

    debug(3, "Reading credentials from %s\n", fullpath);
    const char* opmode = "r";
    FILE* file = NULL;

    if ((file = fopen(fullpath, opmode)) == NULL) {
        debug(1, "fopen(%s) no file\n", fullpath);
        return false;
    }

    size_t buf_size = 1024;
    char* buf = (char*)malloc_memset(buf_size);
    size_t ret = fread(buf, sizeof(char), buf_size, file);
    fclose(file);

    debug(4, "fread(%s) %zu bytes\n", fullpath, ret);

    if (ret < 32) {
        // no content was read in file or it seems corrupt
        // remove it so it gets re-created on next app launch
        debug(1, "No content was read or auth file corrupt? Removing %s\n", fullpath);
        fullpath = creds_file_path(false, true); // remove file
        free(buf);
        return false;
    }

    char* strtok_saveptr = NULL;
    char* line = strtok_r(buf, "\n", &strtok_saveptr);
    while (line != NULL) {
        if (sscanf(line, "cookie = %99s\n", session->cookie) == 1) { // read no more than 99 bytes
            debug(3, "Scanned cookie %s\n", session->cookie);
        }

        for (size_t i = 0; i <= MAX_NUM_OF_BOARDS; ++i) {
            if (!strlen(session->moder.boards[i]) && sscanf(line, "board = %15s\n", session->moder.boards[i]) == 1) { // read no more than 15 bytes
                debug(3, "Scanned board %s\n", session->moder.boards[i]);
                break;
            }
        }
        line = strtok_r(NULL, "\n", &strtok_saveptr);
    }

    debug(3, "Populated credentials cookie: %s\n", session->cookie);
    free(buf);
    return true;
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

static bool _Gui_DrawMainScreen(GuiRuntimeConfig* my_app_config, session_t* session)
{
    assert(my_app_config->window);

    GtkWidget *box = NULL, *button = NULL, *grid = NULL, *label = NULL;

    _Gui_CleanMainChildren(my_app_config->window);

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

    if (!strlen(session->cookie)) {
        // File is present - read it
        // Populate cookie and boards
        char* fullpath = creds_file_path(false, false);
        assert(fullpath);

        if (!_Gui_PopulateSessionFromFile(fullpath, session)) {
            free(fullpath);
            return false;
        }
        free(fullpath);
    }

    for (size_t i = 0; i < sizeof(session->moder.boards) / sizeof(*session->moder.boards); ++i) {
        char* board_name = (char*)session->moder.boards + (MAX_BOARD_NAME_LENGTH * i);
        if (strlen(board_name)) {
            debug(3, "Adding button for (%zu) %s", i, board_name);
            button = gtk_button_new_with_label(board_name);
            assert(button);

            // Fetch thread
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
    GtkWidget *box = data, *grid = NULL, *entry = NULL, *label = NULL;

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

        entry = gtk_entry_new();
        assert(entry);
        gtk_grid_attach(GTK_GRID(grid), entry, 0, 4, 1, 1);
        gtk_widget_set_name(entry, "username_entry");

        static size_t usermode = Username;
        g_signal_connect(GTK_ENTRY(entry), "activate", G_CALLBACK(_Gui_GetText), &usermode);

        label = gtk_label_new("Password");
        assert(label);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
        gtk_label_set_line_wrap(GTK_LABEL(label), FALSE);
        gtk_grid_attach(GTK_GRID(grid), label, 0, 5, 1, 1);
        gtk_widget_set_name(label, "password_label");

        entry = gtk_entry_new();
        assert(entry);
        gtk_grid_attach(GTK_GRID(grid), entry, 0, 6, 1, 1);
        gtk_widget_set_name(entry, "password_entry");

        static size_t passmode = Password;
        g_signal_connect(GTK_ENTRY(entry), "activate", G_CALLBACK(_Gui_GetText), &passmode);

        pressed = true;

        // redraw
        gtk_widget_show_all(box);
    }
}
