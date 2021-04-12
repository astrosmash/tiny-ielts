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

    gui_runtime_config* my_app_config = malloc(sizeof(gui_runtime_config));
    assert(my_app_config);
    memset(my_app_config, 0, sizeof(*my_app_config));

    my_app_config->my_config = config;
    my_app_config->my_gui = g;

    // Start initialization
    GtkWidget *window = NULL;

    // Main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    assert(window);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_window_set_title(GTK_WINDOW(window), "2ch-mod");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect_swapped(window, "delete_event", G_CALLBACK(Gui_Exit), my_app_config);

    if (check_local_account() == 0) {
        _Gui_DrawMainScreen(window, my_app_config);
    } else {
        _Gui_DrawLoginScreen(window, my_app_config);
    }

    gtk_widget_show_all(window);

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

    const char* text = NULL;
    text = gtk_entry_get_text(entry);
    assert(text);

    if (strlen(text)) {
        debug("read %s\n", text);
        g_print("%s \n", text);
    }

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
}

static void _Gui_DrawMainScreen(GtkWidget* window, gui_runtime_config* my_app_config)
{
    assert(window);
    GtkWidget *box = NULL, *entry = NULL, *grid = NULL, *button = NULL, *scroll = NULL;

    // Main widget
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    assert(box);
    gtk_container_set_border_width(GTK_CONTAINER(box), 7);
    gtk_container_add(GTK_CONTAINER(window), box);

    // Text input
    entry = gtk_entry_new();
    assert(entry);
    gtk_container_add(GTK_CONTAINER(box), entry);
    g_signal_connect(GTK_ENTRY(entry), "activate", G_CALLBACK(_Gui_GetText), NULL);

    scroll = gtk_scrolled_window_new(NULL, NULL);
    assert(scroll);
    g_object_set(scroll, "shadow-type", GTK_SHADOW_IN, NULL);
    gtk_container_set_border_width(GTK_CONTAINER(scroll), 17);
    gtk_container_add(GTK_CONTAINER(box), scroll);

    grid = gtk_grid_new();
    assert(grid);
    gtk_container_add(GTK_CONTAINER(box), grid);

    button = gtk_button_new_with_label("Start");
    assert(button);
    g_signal_connect(button, "clicked", G_CALLBACK(_Gui_RunChildThread), my_app_config);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 0, 1, 1);

    button = gtk_button_new_with_label("Stop");
    assert(button);
    g_signal_connect(button, "clicked", G_CALLBACK(_Gui_RunChildThread), my_app_config);
    gtk_grid_attach(GTK_GRID(grid), button, 1, 0, 1, 1);

    button = gtk_button_new_with_label("Quit");
    assert(button);
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(Gui_Exit), my_app_config);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 1, 2, 1);

    button = gtk_button_new_with_label("Join thread");
    assert(button);
    g_signal_connect(button, "clicked", G_CALLBACK(Gui_JoinThread), my_app_config);
    gtk_grid_attach(GTK_GRID(grid), button, 2, 2, 2, 2);
}

static void _Gui_DrawLoginScreen(GtkWidget* window, gui_runtime_config* my_app_config)
{
    assert(window);
    GtkWidget *box = NULL, *entry = NULL, *grid = NULL, *button = NULL, *scroll = NULL;

    // Main widget
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    assert(box);
    gtk_container_set_border_width(GTK_CONTAINER(box), 7);
    gtk_container_add(GTK_CONTAINER(window), box);

    // Text input
    entry = gtk_entry_new();
    assert(entry);
    gtk_container_add(GTK_CONTAINER(box), entry);
    g_signal_connect(GTK_ENTRY(entry), "activate", G_CALLBACK(_Gui_GetText), NULL);

    grid = gtk_grid_new();
    assert(grid);
    gtk_container_add(GTK_CONTAINER(box), grid);

    button = gtk_button_new_with_label("Start");
    assert(button);
    g_signal_connect(button, "clicked", G_CALLBACK(_Gui_RunChildThread), my_app_config);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 0, 1, 1);

    button = gtk_button_new_with_label("Stop");
    assert(button);
    g_signal_connect(button, "clicked", G_CALLBACK(_Gui_RunChildThread), my_app_config);
    gtk_grid_attach(GTK_GRID(grid), button, 1, 0, 1, 1);

    button = gtk_button_new_with_label("Quit");
    assert(button);
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(Gui_Exit), my_app_config);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 1, 2, 1);
}
