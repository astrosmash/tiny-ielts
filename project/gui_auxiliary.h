static void _Gui_RunChildThread(GtkWidget*, gpointer);
static bool _Gui_PopulateSessionFromFile(const char*, session_t*);

// -------------------------------------------------- Drawings
static void _Gui_CleanMainChildren(GtkWidget*);
static bool _Gui_DrawMainScreen(GuiRuntimeConfig*, session_t*);
static void _Gui_DrawLoginInvitationScreen(GuiRuntimeConfig*);
static void _Gui_WantAuthenticate(GtkWidget*, gpointer);

#include "gui_auxiliary.c"
