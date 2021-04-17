// Auxilliary data structures
const char* client_whitelisted_users[] = { "Alexandr", "Shamil", "shamil1989" };

typedef enum {
    false,
    true
} bool;

struct curl_string {
    char* ptr;
    size_t len;
};

extern const char* get_homedir(void);
extern char* creds_file_path(bool, bool);

static size_t curl_write_func(void*, size_t, size_t, struct curl_string*);
static CURL* dvach_curl_init(struct curl_string*, const char*);

#include "auxiliary.c"
