// Global vars
gui_runtime_config* my_app_config = NULL;
static session_t session = { 0 };


// Gui ctor & dtor
#define Gui_Init (*Gui_Construct)

Gui* Gui_Construct(config_t* config)
{
    Gui* g = malloc(sizeof(Gui));
    assert(g);
    assert(config);
    memset(g, 0, sizeof(*g));

    _Gui_SetName(g, "myggtk_gui");
    debug("allocated new object on %p\n", (void*)g);

    my_app_config = malloc(sizeof(gui_runtime_config));
    assert(my_app_config);
    memset(my_app_config, 0, sizeof(*my_app_config));

    // Start initialization
    GtkWidget* window = NULL;

    // Main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    assert(window);
    gtk_widget_set_name(window, "main_window");

    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_window_set_title(GTK_WINDOW(window), "2ch-mod");
    
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect_swapped(window, "delete_event", G_CALLBACK(Gui_Exit), my_app_config);

    my_app_config->my_config = config;
    my_app_config->my_gui = g;
    my_app_config->window = window;

    if (check_local_account() == 0) {
        _Gui_DrawMainScreen();
    } else {
        _Gui_DrawLoginInvitationScreen();
    }

    gtk_widget_show_all(my_app_config->window);

    return g;
}

void Gui_Destruct(Gui** g)
{
    assert(*g);
    debug("freed object on %p\n", (void*)*g);
    free(*g);
    *g = NULL;
}

// Public methods
// Setters

static void Gui_SetApp(Gui* const g, GtkApplication* app)
{
    g->app = app;
}

static void Gui_SetUserData(Gui* const g, gpointer user_data)
{
    g->user_data = user_data;
}

// Getters

GtkApplication* Gui_GetApp(Gui* const g)
{
    return g->app;
}

gpointer Gui_GetUserData(Gui* const g)
{
    return g->user_data;
}

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

// Callback for exit button that calls dtor

static void Gui_Exit(gpointer data, GtkWidget* widget)
{
    assert(data);
    gui_runtime_config* g_config = data;
    debug("freeing %s\n", Gui_GetName(g_config->my_gui));
    Gui_Destruct(&g_config->my_gui);
    gtk_main_quit();
}

//static void Gui_RunChildThread(GtkWidget* widget, gpointer data)
//{
//    gui_runtime_config* g_config = data;
//    GThread* thread = NULL;
//
//    thread = g_thread_new("worker", _Gui_RunChildThread, g_config);
//    assert(thread);
//
//    debug("launching thread %s\n", Gui_GetName(g_config->my_gui));
//    g_thread_unref(thread);
//}

static void Gui_JoinThread(GtkWidget* widget, gpointer data)
{
    assert(data);
    gui_runtime_config* g_config = data;
    assert(g_config->child_thread);
    debug("joining thread at %s\n", Gui_GetName(g_config->my_gui));
    void* res = NULL;

    if (Thread_Join(g_config->child_thread, &res) == 0) {
        debug("thread joined ok res %s\n", (char*)res);
    }
}

static void* thread_func(void* data)
{
    //    do_network(data, 0);
    return "thread_func launched ok";
}

static void* _Gui_RunChildThread(GtkWidget* widget, gpointer data)
{
    assert(data);
    gui_runtime_config* g_config = data;
    assert(g_config->my_gui);
    Thread* my_thread = NULL;

    if (((my_thread = Thread_Init(&thread_func, g_config->my_config)) == NULL)) {
        debug("cannot launch thread_func! %s\n", Gui_GetName(g_config->my_gui));
    }
    g_config->child_thread = my_thread;
    //    do_network(my_config, 0);
    return NULL;
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
        static session_creds_t creds = { 0 };
        if (text_type == FilePath) {
            debug("read %s\n", text);
            g_print("%s \n", text);

            // Does a file exist?
            if (check_local_file(text) == 0) {
                // Try to read it
                char* file = NULL;
                if ((file = read_file(text)) == NULL) {
                    debug("was not able to read %s\n", text);
                    return;
                }
                debug("was able to read %s %s\n", text, file);
                free(file);
            }
        } else if (text_type == Username) {
            // Make sure no overflow occurs
            assert(text_len < MAX_CRED_LENGTH);
            debug("read username %s\n", text);
            g_print("%s \n", text);

            if (strlen(creds.username)) {
                memset(creds.username, 0, MAX_CRED_LENGTH);
            }

            strncpy(creds.username, text, text_len);
        } else if (text_type == Password) {
            // Make sure no overflow occurs
            assert(text_len < MAX_CRED_LENGTH);
            debug("read password %s\n", text);
            g_print("%s \n", text);

            if (strlen(creds.password)) {
                memset(creds.password, 0, MAX_CRED_LENGTH);
            }

            strncpy(creds.password, text, text_len);
        } else {
            debug("text_type unknown %zu, doing nothing\n", text_type);
        }

        if (strlen(session.cookie) == 0) {
            if (strlen(creds.username) && strlen(creds.password)) {
                debug("triggering session_init with user %s pass %s\n", creds.username, creds.password);

                size_t allowed = 0;
                for (size_t i = 0; i < ARRAY_SIZE(client_whitelisted_users); i++) {
                    if (strcmp(*(client_whitelisted_users + i), creds.username) == 0) {
                        allowed = 1;
                    }
                }
                if (!allowed) { debug("you are not whitelisted to use this client! %s", creds.username); return; }

                ssize_t session_res = 0;
                if ((session_res = session_init(&creds, &session))) {
                    debug("was not able to trigger session_init with user %s pass %s (%zd)\n", creds.username, creds.password, session_res);
                    return;
                }

                _Gui_DrawMainScreen();
                gtk_widget_show_all(my_app_config->window);

            }
        } else {
            debug("will not trigger session_init - have session present cookie %s user %s pass %s. Just re-launch an app to relogin\n", session.cookie, session.creds->username, session.creds->password);
        }
    }
}

static void _Gui_DrawMainScreen()
{
    assert(my_app_config->window);
    GtkWidget *box = NULL, *entry = NULL, *grid = NULL, *button = NULL, *scroll = NULL, *label = NULL;

    if (GTK_IS_CONTAINER(my_app_config->window)) {
        GList* children = gtk_container_get_children(GTK_CONTAINER(my_app_config->window));
        fprintf(stdout, "checking child containers...\n");

        for (const GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
            fprintf(stdout, "FOUND child...\n");
            gtk_widget_destroy(GTK_WIDGET(iter->data));
        }
    }

    // Main widget
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    assert(box);
    gtk_container_set_border_width(GTK_CONTAINER(box), 7);
    gtk_container_add(GTK_CONTAINER(my_app_config->window), box);
    gtk_widget_set_name(box, "main_box");

    // Text input
    entry = gtk_entry_new();
    assert(entry);
    gtk_container_add(GTK_CONTAINER(box), entry);
    gtk_widget_set_name(entry, "main_text_input");

    static size_t mode = FilePath;
    g_signal_connect(GTK_ENTRY(entry), "activate", G_CALLBACK(_Gui_GetText), &mode);

    scroll = gtk_scrolled_window_new(NULL, NULL);
    assert(scroll);
    g_object_set(scroll, "shadow-type", GTK_SHADOW_IN, NULL);
    gtk_container_set_border_width(GTK_CONTAINER(scroll), 17);
    gtk_container_add(GTK_CONTAINER(box), scroll);
    gtk_widget_set_name(scroll, "main_scroll");

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

    for (size_t i = 0; i < sizeof(session.moder.boards) / sizeof(*session.moder.boards); ++i) {
        char* board_name = session.moder.boards[i];
        if (strlen(board_name)) {
           debug("Adding button for (%u) %s", i, (char*)session.moder.boards + ( MAX_BOARD_NAME_LENGTH * i ));

           button = gtk_button_new_with_label(board_name);
           assert(button);
           g_signal_connect(button, "clicked", G_CALLBACK(_Gui_RunChildThread), my_app_config);
           gtk_grid_attach(GTK_GRID(grid), button, 0, i+1, i+1, i+1);
           gtk_widget_set_name(button, board_name);
        }
    }

}

static void _Gui_DrawLoginInvitationScreen()
{
    assert(my_app_config->window);
    GtkWidget *box = NULL, *grid = NULL, *button = NULL, *label = NULL;

    // Main widget
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    assert(box);
    gtk_container_set_border_width(GTK_CONTAINER(box), 2);
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
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(Gui_Exit), my_app_config);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 1, 1);
    gtk_widget_set_name(button, "quit_button");

}

static void _Gui_WantAuthenticate(GtkWidget* widget, gpointer data)
{
    assert(data);
    GtkWidget *box = data, *grid = NULL, *label = NULL, *entry = NULL;

    // For the program lifetime - we do not want to add form on each click
    static size_t pressed = 0;

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

        pressed = 1;

        // redraw
        gtk_widget_show_all(box);
    }
}
