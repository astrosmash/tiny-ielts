// Include debug output
#define DEBUG 1

extern size_t check_local_account(void);
extern size_t check_local_file(const char*);

#define MAX_CRED_LENGTH 99

typedef struct {
    char username[MAX_CRED_LENGTH];
    char password[MAX_CRED_LENGTH];
} session_creds_t;

// Definition
#include "2ch.c"
