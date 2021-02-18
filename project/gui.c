config_t* my_config = NULL;

ssize_t gui_init(int argc, char** argv, config_t* config)
{
    GtkApplication* app;
    app = gtk_application_new("org.gtk.tinyIelts", G_APPLICATION_FLAGS_NONE);
    assert(app);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    my_config = config;
    ssize_t gui_status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return gui_status;
}

void activate(GtkApplication* app, gpointer user_data)
{
    GtkWidget* window = NULL;
    GtkWidget* button = NULL;

    window = gtk_application_window_new(app);
    assert(window);
    gtk_window_set_title(GTK_WINDOW(window), "tiny-ielts");
    gtk_window_set_default_size(GTK_WINDOW(window), 1360, 1180);

    button = gtk_button_new_with_label("start");
    assert(button);
    GtkStyleContext* context = gtk_widget_get_style_context(button);
    GtkCssProvider* provider = gtk_css_provider_new();
    assert(context);
    assert(provider);
    const char* button_css = "button {background-image: none; background-color: rgb(255, 255, 0); color: green;}";
    gtk_css_provider_load_from_data(provider, button_css, strlen(button_css));
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);

    gtk_window_set_child(GTK_WINDOW(window), button);
    gtk_window_present(GTK_WINDOW(window));
}

void print_hello(void)
{
    fprintf(stdout, "button clicked...\n");
    g_print("button clicked...\n");
    do_network(my_config, 0);
}