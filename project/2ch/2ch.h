#define MAX_ARBITRARY_CHAR_LENGTH 512
#define MAX_BOARD_NAME_LENGTH 10
#define MAX_CRED_LENGTH 99

#define MAX_NUM_OF_BOARDS 40
#define MAX_NUM_OF_FILES 512
#define MAX_NUM_OF_JIDS 10
#define MAX_NUM_OF_THREADS 512

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


// Definition
#include "2ch.c"
