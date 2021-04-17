// Parser
static void dvach_populate_session(session_t*, cJSON*);
static void dvach_populate_board(board_t*, cJSON*);

// TODO
//extern post_t* fetch_all_posts_from_board(session_t*, board_t*);
//extern thread_t* fetch_thread_from_board(session_t*, board_t*);
//extern post_t* fetch_post_from_thread(session_t*, thread_t*);
//extern file_t* fetch_file_from_post(session_t*, post_t*);

#include "parser.c"
