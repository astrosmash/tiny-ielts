// Data structures
typedef struct {
    char username[MAX_CRED_LENGTH];
    char password[MAX_CRED_LENGTH];
} session_creds_t;

typedef struct {
    char key[MAX_CRED_LENGTH];
    char gid[MAX_CRED_LENGTH];
} spreadsheet_t;

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

    char comment[90 * MAX_ARBITRARY_CHAR_LENGTH];

    char date[MAX_ARBITRARY_CHAR_LENGTH];

    char email[MAX_ARBITRARY_CHAR_LENGTH];

    file_t file[MAX_NUM_OF_FILES]; // TBR - maybe need dynamic allocation

    size_t lasthit;

    size_t posts_count;

    bool sticky;

    char subject[MAX_ARBITRARY_CHAR_LENGTH];

    size_t timestamp;

    size_t parent;

    bool op;

    // Modinfo
    char country[3];

    char ip[16];

} post_t;

typedef post_t thread_t;

typedef struct {
    char name[MAX_BOARD_NAME_LENGTH];

    char verbose_name[MAX_ARBITRARY_CHAR_LENGTH];

    size_t bump_limit;

    thread_t thread[MAX_NUM_OF_THREADS]; // TBR - maybe need dynamic allocation

} board_t;

typedef struct {
    char ban_reason[MAX_NUM_OF_BAN_REASONS][MAX_ARBITRARY_CHAR_LENGTH];

    post_t post[MAX_NUM_OF_POSTS_PER_BOARD_AS_MODER];

} board_as_moder_t;
