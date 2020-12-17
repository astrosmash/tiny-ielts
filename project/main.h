#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>

#ifdef __linux__
#include <sys/epoll.h>
#endif

#include "config.h"
#include "network.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
