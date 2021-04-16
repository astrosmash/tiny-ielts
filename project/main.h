#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <cairo.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <gtk/gtk.h>

#ifdef __linux__
#include <sys/epoll.h>
#include <sys/select.h>
#endif

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// Inclusive logging output
// 1: (real) errors
// 2: warnings about possibly not implemented data fields
// 3: verbose info about parsed fields / configurations that were set
// 4: iteration numbers and info about current location in nested loops
// 5: full trace of received content
#define debug(lvl, fmt, ...)                                                                       \
    do {                                                                                      \
        if (DEBUG && DEBUG_LEVEL >= lvl)                                                                            \
            fprintf(stderr, "\n%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } while (0)


#include "2ch.h"
#include "config.h"
#include "gui.h"
#include "network.h"
