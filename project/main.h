#include <assert.h>
#include <ctype.h>
#include <db.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>

#include <cairo.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <gtk/gtk.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// Inclusive logging output
// 1: (real) errors
// 2: warnings about possibly not implemented data fields
// 3: verbose info about parsed fields / configurations that were set
// 4: iteration numbers and info about current location in nested loops
// 5: full trace of received content
#define debug(lvl, fmt, ...)                                                                  \
    do {                                                                                      \
        if (DEBUG && DEBUG_LEVEL >= lvl)                                                      \
            fprintf(stderr, "\n%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } while (0)

// Include debug output
#define DEBUG 1
#define DEBUG_LEVEL 5

#define MAX_ALLOCABLE_MEM_BLOCKS 10240

enum {
    ALLOCATION = 1 << 0,
    REMOVAL = 1 << 1,
    GLOBAL_CLEANUP = 1 << 2
} track_allocated_blocks_flags;

typedef enum {
    false,
    true
} bool;

static bool track_allocated_blocks(void*, size_t);
void* malloc_memset(size_t);
void safe_free(void**);

#include "2ch/2ch.h"
#include "gui.h"
