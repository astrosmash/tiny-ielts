// Auxilliary data structures
const char* client_whitelisted_users[] = { "Alexandr", "Shamil", "shamil1989" };

struct curl_string {
    char* ptr;
    size_t len;
};

struct g_callback_task {
    void* what;
    void* result;
    size_t type;
    GtkWidget* caller_widget;
};

enum {
    REMOVE_POST = 1,
    ADD_LOCAL_BAN,
    WHOIS_POST,
    FILTER_BY_IP_PER_BOARD
} task_types;

enum {
    LOOK_FOR_TASKS = 1 << 0,
    INSERT_TASK = 1 << 1
} task_manager_modes;

enum {
    NEED_TO_CHECK = 1 << 0,
    NEED_TO_CREATE = 1 << 1,
    NEED_TO_DELETE = 1 << 2
} creds_file_modes;

extern const char* get_homedir(void);
extern char* creds_file_path(size_t);
extern void* task_manager(size_t, void*, size_t);
extern struct g_callback_task* create_new_task(size_t, void*);

static size_t curl_write_func(void*, size_t, size_t, struct curl_string*);
static CURL* dvach_curl_init(struct curl_string*, const char*);
static bool submit_curl_task(const char*, const char*, struct curl_string*, void*);

#include "auxiliary.c"
