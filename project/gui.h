#include "thread.h"

// class Gui
#define maxGuiNameLength 255

typedef struct {
    char name[maxGuiNameLength + 1];
} Gui;

typedef struct {
    Gui* my_gui;
    Thread* child_thread;
    // Main window to (re)draw on
    GtkWidget* window;

    struct {
        // Board to draw
        char board[MAX_BOARD_NAME_LENGTH];
        // Session details to use
        session_t session;
    } WorkerData;
} GuiRuntimeConfig;

enum {
    Username = 1,
    Password,
} _Gui_GetText_Type;

// Gui ctor & dtor
Gui* Gui_Construct(void);
void Gui_Destruct(Gui**);

// Getters
static char* Gui_GetName(Gui* const);

// Private methods
static void _Gui_SetName(Gui*, char*);

// Static methods
// Callback for exit button that calls dtor, called with swapped params
static void Gui_Exit(gpointer, GtkWidget*);

static void _Gui_CleanMainChildren(void);

static void _Gui_DrawLoginInvitationScreen(void);
static void _Gui_DrawMainScreen(void);
static void _Gui_GetText(GtkEntry*, gpointer);
static void _Gui_WantAuthenticate(GtkWidget*, gpointer);

static void _Gui_RunChildThread(GtkWidget*, gpointer);
static void _Gui_JoinThread(GtkWidget*, gpointer);

static void* thread_func(void*);

// Definition
#include "gui.c"
