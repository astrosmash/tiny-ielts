#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>

#include <cairo.h>
#include <gtk/gtk.h>

#ifdef __linux__
#include <sys/epoll.h>
#include <sys/select.h>
#endif

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define debug(fmt, ...)                                                                     \
    do {                                                                                    \
        if (DEBUG)                                                                          \
            fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } while (0)

#include "config.h"
#include "network.h"
#include "gui.h"
