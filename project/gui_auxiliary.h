static void _Gui_RunChildThread(GtkWidget*, gpointer);

// -------------------------------------------------- Drawings
static void _Gui_CleanMainChildren(GtkWidget*);
static bool _Gui_DrawPopup(GtkWidget*, GdkEvent*);
static bool _Gui_DrawMainScreen(GuiRuntimeConfig*);
static void _Gui_DrawLoginInvitationScreen(GuiRuntimeConfig*);
static void _Gui_WantAuthenticate(GtkWidget*, gpointer);

#include "gui_auxiliary.c"
