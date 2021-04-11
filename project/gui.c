// Gui ctor & dtor
#define Gui_Init (*Gui_Construct)

Gui* Gui_Construct(void)
{
    Gui* g = malloc(sizeof(Gui));
    assert(g);
    _Gui_SetName(g, "myggtk_gui");
    fprintf(stdout, "class Gui: allocated new object on %p\n", g);
    return g;
}

void Gui_Destruct(Gui* const g)
{
//    assert(g);
    fprintf(stdout, "class Gui: freed object on %p\n", g);
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

Gui* my_gui = NULL;
config_t* my_config = NULL;

ssize_t gui_init(int argc, char** argv, config_t* config)
{
    my_gui = Gui_Init();

    GtkApplication* app;
    app = gtk_application_new("org.gtk.tinyIelts", G_APPLICATION_FLAGS_NONE);
    assert(app);

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    Gui_SetApp(my_gui, app);
    my_config = config;

    ssize_t gui_status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return gui_status;
}

void activate()
{
    GtkWidget* window = NULL;
    GtkWidget* button = NULL;

    window = gtk_application_window_new(Gui_GetApp(my_gui));
    assert(window);
    gtk_window_set_title(GTK_WINDOW(window), "tiny-ielts");
    gtk_window_set_default_size(GTK_WINDOW(window), 360, 180);
    gtk_widget_show_all(window);
//    button = gtk_button_new_with_label("start");
//    assert(button);
//
//    GtkStyleContext* context = gtk_widget_get_style_context(button);
//    GtkCssProvider* provider = gtk_css_provider_new();
//    assert(context);
//    assert(provider);
//
//    const char* button_css = "button {background-image: none; background-color: rgb(255, 255, 0); color: green;}";
//    gtk_css_provider_load_from_data(provider, button_css, strlen(button_css), NULL);
//    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
//    g_object_unref(provider);
//
//    g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);
//
//    gtk_container_add(GTK_CONTAINER(window), button);
//    gtk_window_present(GTK_WINDOW(window));
}

void print_hello(void)
{
    fprintf(stdout, "button clicked...\n");
    fprintf(stdout, "gui name: %s\n", Gui_GetName(my_gui));
    g_print("button clicked...\n");

    ssize_t thread_status = 0;
    if ((thread_status = t_init())) {
        fprintf(stderr, "Cannot launch THREAD = %zd\n", thread_status);
    }

    //    GtkWidget* inside_window = gtk_application_window_new();
    //    assert(inside_window);
    //    gtk_window_set_title(GTK_WINDOW(inside_window), "tiny-ielts");
    //    gtk_window_set_default_size(GTK_WINDOW(inside_window), 665, 400);
    //    do_network(my_config, 0);
    Gui_Destruct(my_gui);
    my_gui = NULL;
}
