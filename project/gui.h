// Include debug output for Gui and Thread objects
#define DEBUG 1

#include "thread.h"

// class Gui
#define maxGuiNameLength 255

typedef struct {
    char name[maxGuiNameLength + 1];
    GtkApplication* app;
    gpointer user_data;
} Gui;

typedef struct {
    config_t* my_config;
    Gui* my_gui;

    struct {
        GtkWidget* window;
        guint* progress;
    } WorkerData;
} gui_runtime_config;

// Gui ctor & dtor
Gui* Gui_Construct(int, char**, config_t*);
void Gui_Destruct(Gui* const);

// Public methods
// Setters
static void Gui_SetApp(Gui* const, GtkApplication*);
static void Gui_SetUserData(Gui* const, gpointer);
// Getters
GtkApplication* Gui_GetApp(Gui* const);
gpointer Gui_GetUserData(Gui* const);
static char* Gui_GetName(Gui* const);

// Private methods
// Setters
static void _Gui_SetName(Gui*, char*);

// Static methods
// Callback for exit button that calls dtor, called with swapped params
static void gui_exit(gpointer, GtkWidget*);
static void run_thread(GtkWidget*, gpointer);
static void* t_print_hello(void*);
static void* print_hello(void*);

// Definition
#include "gui.c"
