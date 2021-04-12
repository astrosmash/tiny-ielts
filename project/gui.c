// Gui ctor & dtor
#define Gui_Init (*Gui_Construct)

Gui* Gui_Construct(int argc, char** argv, config_t* config)
{
    Gui* g = malloc(sizeof(Gui));
    assert(g);

    _Gui_SetName(g, "myggtk_gui");
    debug("allocated new object on %p\n", (void*)g);

    gui_runtime_config* my_app_config = malloc(sizeof(gui_runtime_config));
    assert(my_app_config);

    my_app_config->my_config = config;
    my_app_config->my_gui = g;

    GtkWidget* window = NULL;
    GtkWidget* button = NULL;
    GtkWidget* grid = NULL;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    assert(window);

    gtk_window_set_title(GTK_WINDOW(window), "tiny-ielts");
    //    gtk_window_set_default_size(GTK_WINDOW(window), 360, 180);
    g_signal_connect(window, "delete_event", G_CALLBACK(gui_exit), config);

    grid = gtk_grid_new();
    assert(grid);

    gtk_container_add(GTK_CONTAINER(window), grid);

    button = gtk_button_new_with_label("Start");
    assert(button);
    g_signal_connect(button, "clicked", G_CALLBACK(run_thread), my_app_config);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 0, 1, 1);

    button = gtk_button_new_with_label("Stop");
    assert(button);
    g_signal_connect(button, "clicked", G_CALLBACK(run_thread), my_app_config);
    gtk_grid_attach(GTK_GRID(grid), button, 1, 0, 1, 1);

    button = gtk_button_new_with_label("Quit");
    assert(button);
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(gui_exit), config);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 1, 2, 1);

    gtk_widget_show_all(window);
    return g;
}

void Gui_Destruct(Gui* const g)
{
    debug("freed object on %p\n", (void*)g);
    free(g);
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
    return g->name;
}

// Private methods

static void _Gui_SetName(Gui* g, char* name)
{
    assert(name);
    strncpy(g->name, name, strlen(name));
}

// Callback for exit button that calls dtor

static void gui_exit(gpointer data, GtkWidget* widget)
{
    gui_runtime_config* g_config = data;
    debug("freeing %s\n", Gui_GetName(g_config->my_gui));
    Gui_Destruct(g_config->my_gui);
}

static void run_thread(GtkWidget* widget, gpointer data)
{
    gui_runtime_config* g_config = data;
    GThread* thread = NULL;

    thread = g_thread_new("worker", print_hello, g_config);
    assert(thread);

    debug("launching thread %s\n", Gui_GetName(g_config->my_gui));
    g_thread_unref(thread);
}

static void* t_print_hello(void* data)
{
    gui_runtime_config* g_config = data;
    do_network(g_config->my_config, 0);

    return "launched ok";
    //    Thread_Destruct(g);
    //    g = NULL;
}

static void* print_hello(void* data)
{
    gui_runtime_config* g_config = data;

    g_print("print_hello button clicked...\n");
    Thread* my_thread = NULL;
    if (((my_thread = Thread_Init(&t_print_hello, &g_config->my_config)) == NULL)) {
        debug("cannot launch thread! %s\n", Gui_GetName(g_config->my_gui));
        fprintf(stderr, "[print_hello] cannot launch thread\n");
    }
    //    do_network(my_config, 0);
    return NULL;
}
