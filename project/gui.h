extern ssize_t gui_init(int, char**, config_t*);

extern void activate(GtkApplication*, gpointer);
extern void print_hello(void);

// class Gui
#define maxGuiNameLength 255

typedef struct {
    char name[maxGuiNameLength + 1];
    GtkApplication* app;
    gpointer user_data;
} Gui;

// Gui ctor & dtor
Gui* Gui_Construct(void);
void Gui_Destruct(Gui* const g);

// Public methods
static void Gui_SetApp(Gui* const g, GtkApplication* app);
static void Gui_SetUserData(Gui* const g, gpointer user_data);
GtkApplication* Gui_GetApp(Gui* const g);
gpointer Gui_GetUserData(Gui* const g);
static char* Gui_GetName(Gui* const g);

// Private methods
static void _Gui_SetName(Gui* g, char* name);

// Definition
#include "gui.c"
