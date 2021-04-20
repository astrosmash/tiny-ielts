#include "thread.h"
// class Gui
#define maxGuiNameLength 255

typedef struct {
    char name[maxGuiNameLength + 1];
} Gui;

typedef struct {
    Gui* my_gui;
    Thread* board_top_fetch_thread;
    Thread* monitor_thread;
    // Main window to (re)draw on
    GtkWidget* window;

    struct {
        // Board to draw
        char board[MAX_BOARD_NAME_LENGTH];
        // Session details to use
        session_t* session;
    } WorkerData;
} GuiRuntimeConfig;

#define NUMBER_OF_AUTH_FORMS 2
struct GtkEntries {
    GtkWidget* entry[NUMBER_OF_AUTH_FORMS];
};

// Gui ctor & dtor
Gui* Gui_Construct(void);
void Gui_Destruct(Gui**);

// Getters
static char* Gui_GetName(Gui* const);

// Private methods
static void _Gui_SetName(Gui*, char*);

// Callback for exit button that calls dtor, called with swapped params
static void _Gui_Exit(gpointer, GtkWidget*);
static void _Gui_GetText(GtkEntry*, gpointer);

#include "gui_auxiliary.h"

// Definition
#include "gui.c"
