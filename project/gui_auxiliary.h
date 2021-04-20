extern GuiRuntimeConfig* get_gui_runtime_config(bool);
extern session_t* get_session(bool);
extern session_creds_t* get_session_creds(bool);

static gboolean update_gui(gpointer);
static void popup_minimal(GtkWidget*, const char*);
extern void* task_monitor(void*);
extern void* worker_func(void*);
// -------------------------------------------------- Threading
static void _Gui_RunBoardTopFetchThread(GtkWidget*, gpointer);
// -------------------------------------------------- Drawings
static void _Gui_CleanMainChildren(GtkWidget*);
static void _Gui_DrawPopupMenu(GtkWidget*, GdkEvent*);
static void _Gui_DrawPopupDialog(GtkWidget*, void*);
static bool _Gui_DrawMainScreen(GuiRuntimeConfig*);
static void _Gui_DrawLoginInvitationScreen(GuiRuntimeConfig*);
static void _Gui_DrawAuthScreen(GtkWidget*, gpointer);
#include "gui_auxiliary.c"
