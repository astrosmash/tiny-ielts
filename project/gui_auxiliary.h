// -------------------------------------------------- Drawings
static void _Gui_CleanMainChildren(GtkWidget*);
static void _Gui_DrawAuthScreen(GtkWidget*, gpointer);
static void _Gui_DrawLoginInvitationScreen(GuiRuntimeConfig*);
static bool _Gui_DrawMainScreen(GuiRuntimeConfig*);
static void _Gui_DrawPopupMenu(GtkWidget*, GdkEvent*);
static void _Gui_DrawPopupDialog(GtkWidget*, void*);
// -------------------------------------------------- Threading
extern void* task_monitor(void*);

static void _Gui_RunBoardTopFetchThread(GtkWidget*, gpointer);
static gboolean update_gui(gpointer);

#include "gui_auxiliary.c"
