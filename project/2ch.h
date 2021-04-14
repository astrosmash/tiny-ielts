// Include debug output
#define DEBUG 1

// Data structures
#define MAX_CRED_LENGTH 99
#define MAX_BOARD_NAME_LENGTH 10
#define MAX_ARBITRARY_CHAR_LENGTH 512
#define MAX_NUM_OF_JIDS 10
#define MAX_NUM_OF_BOARDS 40

struct curl_string {
    char* ptr;
    size_t len;
};

typedef struct {
    char username[MAX_CRED_LENGTH];
    char password[MAX_CRED_LENGTH];
} session_creds_t;

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

        enum {
            true = 1,
            false
        } public_log;

        size_t last_login;

        size_t last_action;

    } moder;

} session_t;

enum {
    Username = 1,
    Password,
    FilePath
} _Gui_GetText_Type;

const char* client_whitelisted_users[] = {"Alexandr", "Shamil", "shamil1989"};

// Functions and methods
extern size_t check_local_account(void);
extern size_t check_local_file(const char*);
extern ssize_t session_init(session_creds_t*, session_t*);

static size_t curl_write_func(void*, size_t, size_t, struct curl_string*);

// Definition
#include "2ch.c"
