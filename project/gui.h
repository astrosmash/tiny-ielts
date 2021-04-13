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
    Thread* child_thread;

    struct {
        GtkWidget* window;
        guint* progress;
    } WorkerData;
} gui_runtime_config;

// Gui ctor & dtor
Gui* Gui_Construct(config_t*);
void Gui_Destruct(Gui**);

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
// Callback for exit button that calls dtor, called with swapped params
static void Gui_Exit(gpointer, GtkWidget*);
//static void Gui_RunChildThread(GtkWidget*, gpointer);
static void Gui_JoinThread(GtkWidget*, gpointer);
static void* _Gui_RunChildThread(GtkWidget*, gpointer);
static void _Gui_GetText(GtkEntry*, gpointer);

static void _Gui_DrawLoginScreen(GtkWidget*, gui_runtime_config*);
static void _Gui_DrawLoginInvitationScreen(GtkWidget*, gui_runtime_config*);
static void _Gui_DrawMainScreen(GtkWidget*, gui_runtime_config*);
static void _Gui_WantAuthenticate(GtkWidget*, gpointer);

static void* thread_func(void*);

// Definition
#include "gui.c"
