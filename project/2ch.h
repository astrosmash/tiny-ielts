// Include debug output
#define DEBUG 1

// Data structures
#define MAX_CRED_LENGTH 99
#define MAX_BOARD_NAME_LENGTH 10
#define MAX_ARBITRARY_CHAR_LENGTH 512
#define MAX_NUM_OF_JIDS 10
#define MAX_NUM_OF_BOARDS 40
#define MAX_NUM_OF_THREADS 512
#define MAX_NUM_OF_FILES 512

struct curl_string {
    char* ptr;
    size_t len;
};

typedef struct {
    char username[MAX_CRED_LENGTH];
    char password[MAX_CRED_LENGTH];
} session_creds_t;

typedef enum {
    true = 1,
    false
} bool;

typedef struct {
    session_creds_t* creds;

    char cookie[MAX_ARBITRARY_CHAR_LENGTH];

    char telegram_key[MAX_ARBITRARY_CHAR_LENGTH];

    struct {
        size_t num;

        char nick[MAX_CRED_LENGTH]; // creds->username

        size_t level;

        char request_message[MAX_ARBITRARY_CHAR_LENGTH];

        char email[MAX_ARBITRARY_CHAR_LENGTH];

        size_t telegram_id;

        char telegram_username[MAX_ARBITRARY_CHAR_LENGTH];

        char jids[MAX_NUM_OF_JIDS][MAX_CRED_LENGTH];

        char comment[MAX_ARBITRARY_CHAR_LENGTH];

        char boards[MAX_NUM_OF_BOARDS][MAX_BOARD_NAME_LENGTH];

        bool public_log;

        size_t last_login;

        size_t last_action;

    } moder;

} session_t;

typedef struct {
    char displayname[MAX_ARBITRARY_CHAR_LENGTH];

    char fullname[MAX_ARBITRARY_CHAR_LENGTH];

    size_t height;

    size_t width;

    char md5[MAX_ARBITRARY_CHAR_LENGTH];

    char name[MAX_ARBITRARY_CHAR_LENGTH];

    bool nsfw;

    char path[MAX_ARBITRARY_CHAR_LENGTH];

    size_t size;

    char thumbnail[MAX_ARBITRARY_CHAR_LENGTH];

    size_t tn_height;

    size_t tn_width;

    size_t type;

} file_t;

typedef struct {
    size_t num;

    bool banned;

    bool closed;

    char comment[MAX_ARBITRARY_CHAR_LENGTH];

    char date[MAX_ARBITRARY_CHAR_LENGTH];

    char email[MAX_ARBITRARY_CHAR_LENGTH];

    file_t file[MAX_NUM_OF_FILES];

    size_t lasthit;

    size_t posts_count;

    bool sticky;

    char subject[MAX_ARBITRARY_CHAR_LENGTH];

    size_t timestamp;

    size_t parent;

    bool op;

} post_t;

typedef post_t thread_t;

typedef struct {
    char name[MAX_BOARD_NAME_LENGTH];

    char verbose_name[MAX_ARBITRARY_CHAR_LENGTH];

    size_t bump_limit;

    thread_t thread[MAX_NUM_OF_THREADS]; // TBR - maybe need dynamic allocation

} board_t;

extern board_t* fetch_board_info(session_t*, const char*);

extern thread_t* fetch_thread_from_board(session_t*, board_t*);

extern post_t* fetch_post_from_thread(session_t*, thread_t*);

extern file_t* fetch_file_from_post(session_t*, post_t*);

const char* client_whitelisted_users[] = { "Alexandr", "Shamil", "shamil1989" };

// Functions and methods
extern size_t check_local_account(void);
extern size_t check_local_file(const char*);
extern ssize_t session_init(session_creds_t*, session_t*);

static size_t curl_write_func(void*, size_t, size_t, struct curl_string*);

// Definition
#include "2ch.c"
