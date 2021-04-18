#define MAX_ARBITRARY_CHAR_LENGTH 512
#define MAX_BOARD_NAME_LENGTH 10
#define MAX_CRED_LENGTH 99

#define MAX_NUM_OF_BOARDS 40
#define MAX_NUM_OF_FILES 512
#define MAX_NUM_OF_JIDS 10
#define MAX_NUM_OF_THREADS 512
#define MAX_NUM_OF_BAN_REASONS 50
#define MAX_NUM_OF_POSTS_PER_BOARD_AS_MODER 200

// Helper data structures and functions
#include "auxiliary.h"

// Data structures
#include "structs.h"

// Functions to populate allocated data structures
#include "parser.h"

// Functions and methods
// External
extern bool populate_session_from_file(session_t*);
extern bool populate_file_from_session(session_creds_t*, session_t*);
extern ssize_t session_init(session_creds_t*, session_t*);
extern board_t* fetch_board_info(session_t*, const char*);
extern board_as_moder_t* fetch_board_info_as_moder(session_t*, const char*);

extern void remove_post(GtkWidget*, gpointer);
extern void add_local_ban(GtkWidget*, gpointer);
extern void whois(GtkWidget*, gpointer);
extern void filter_by_ip_per_board(GtkWidget*, gpointer);

// TODO
//extern post_t* fetch_all_posts_from_board(session_t*, board_t*);
//extern thread_t* fetch_thread_from_board(session_t*, board_t*);
//extern post_t* fetch_post_from_thread(session_t*, thread_t*);
//extern file_t* fetch_file_from_post(session_t*, post_t*);

// Definition
#include "2ch.c"
