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
    if (((my_thread = Thread_Init(&thread_func, g_config)) == NULL)) {
        debug(1, "Cannot launch thread_func! %s\n", Gui_GetName(g_config->my_gui));
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

gboolean update_gui(gpointer data)
{
    struct g_callback_task* task = data;
    GtkWidget* parent = task->caller_widget;
    _Gui_DrawPopupDialog(parent, task->result);
    gtk_widget_queue_draw(parent);
    safe_free(&task->result);
    return FALSE; //  If the function returns FALSE it is automatically removed from the list of event sources and will not be called again.
}

extern void* task_monitor(void* runtime_config)
{
    //    assert(runtime_config);
    //    GuiRuntimeConfig* config = runtime_config;

    struct timespec iter_retry = { .tv_sec = 1 };
    struct g_callback_task* task = NULL;
    task = malloc_memset(sizeof(struct g_callback_task));

    while (true) {
        for (size_t task_type = REMOVE_POST; task_type <= FILTER_BY_IP_PER_BOARD; ++task_type) {
            if (((task = task_manager(LOOK_FOR_TASKS, NULL, task_type))) != NULL) { // We have a result, let's draw it
                debug(1, "Got task result to draw: %s", task->result);
                g_idle_add(update_gui, task);
            } else {
                debug(1, "Waiting for result to draw - %s", "sleeping...");
            }
        }
        nanosleep(&iter_retry, NULL);
    }
    return NULL;
}
