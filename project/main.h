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

#include <gtk/gtk.h>
#include <cairo.h>

#ifdef __linux__
#include <sys/epoll.h>
#include <sys/select.h>
#endif

#include "config.h"
#include "network.h"
#include "gui.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
