extern size_t set_sock_opts(size_t);
extern size_t read_from_socket(size_t, char*, size_t);
extern size_t write_select_fd(size_t);
#ifdef __linux__
extern size_t write_epoll_fd(struct epoll_event*);
#endif
extern size_t do_network(config_t*, size_t);

#include "network.c"
