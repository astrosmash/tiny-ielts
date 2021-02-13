size_t set_sock_opts(size_t sockfd, size_t set_reuse)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) == -1) {
        fprintf(stderr, "fcntl() -1 %s\n", strerror(errno));
        return 1;
    }
    if (set_reuse) {
        size_t one = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
            fprintf(stderr, "setsockopt() -1 %s\n", strerror(errno));
            return 1;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one)) == -1) {
            fprintf(stderr, "setsockopt() -1 %s\n", strerror(errno));
            return 1;
        }
    }
    return 0;
}


void alloc_read_write_buf(char *r, char* w, size_t size) {
    fprintf(stdout, "allocating buffers\n");
    r = (char*)malloc(size + 1);
    w = (char*)malloc(size + 1);
    assert(r);
    assert(w);
    memset(r, 0, size + 1);
    memset(w, 0, size + 1);
}

size_t read_fd(size_t sockfd, char* read_buf, size_t read_buf_size)
{
    ssize_t nread = 0;
    nread = read(sockfd, read_buf, read_buf_size);

    if (nread == -1 && (errno == EAGAIN)) {
        return 1;
    }

    if (nread < 0 && (errno == EINTR || errno == EWOULDBLOCK)) {
        return 0;
    }

    if (nread < 0) {
        fprintf(stderr, "read_fd() read() %s\n", strerror(errno));
        return 1;
    }

    if (nread == 0) {
        fprintf(stdout, "read_fd() read() closed socket\n");
        return 0;
    }

    fprintf(stdout, "read_fd() read_buf() %s\n", read_buf);
    fflush(stdout);
    return nread;
}

size_t strtok_read(char* buf)
{
    size_t ret = 0;
    char* strtok_saveptr = NULL;
    char* tok = strtok_r(buf, " ", &strtok_saveptr);
    while (tok != NULL) {
        ret = 1;
        fprintf(stdout, "%s\n", tok);
        tok = strtok_r(NULL, " ", &strtok_saveptr);
    }
    return ret;
}

size_t write_no_epoll_fd(size_t sockfd, struct sockaddr* my_addr,
    socklen_t sockaddr_size, void* data)
{
    assert(data);
    size_t ret = 0;
    ssize_t accept_sockfd = 0;
    struct timespec tm = { .tv_sec = 1 };

    for (;;) { // Loop
        ssize_t num_ready = 0;
        fd_set readfds, writefds = { 0 };
        struct timeval timeout = { .tv_sec = 1, .tv_usec = 0 };
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_SET(sockfd, &readfds);
        FD_SET(sockfd, &writefds);
        if ((num_ready = select(FD_SETSIZE, &readfds, &writefds, NULL, &timeout)) == 0) {
            // Timeout
            fprintf(stderr, "select() 1 sec timeout");
            continue;
        }
        if (num_ready == -1) {
            fprintf(stderr, "select() -1 %s\n", strerror(errno));
            ret = 1;
            break;
        }
        fprintf(stdout, "select() got %zd\n", num_ready);
        if (FD_ISSET(sockfd, &readfds)) {
            fprintf(stdout, "sockfd ready for reading\n");
            if ((accept_sockfd = accept(sockfd, my_addr, &sockaddr_size)) == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    nanosleep(&tm, NULL);
                    continue;
                }
                fprintf(stderr, "accept() -1 %s\n", strerror(errno));
                ret = 1;
                break;
            }
        }
        if (set_sock_opts(accept_sockfd, 0)) {
            fprintf(stderr, "fcntl(accept) -1 %s\n", strerror(errno));
            ret = 1;
            break;
        }

        fprintf(stdout, "write_no_epoll_fd(%zu) accept %zd\n", sockfd,
            accept_sockfd);

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_SET(sockfd, &readfds);
        FD_SET(sockfd, &writefds);
        FD_SET(accept_sockfd, &readfds);
        FD_SET(accept_sockfd, &writefds);
        if ((num_ready = select(FD_SETSIZE, &readfds, &writefds, NULL, &timeout)) == 0) {
            // Timeout
            fprintf(stderr, "select() 1 sec timeout");
            continue;
        }
        if (num_ready == -1) {
            fprintf(stderr, "select() -1 %s\n", strerror(errno));
            ret = 1;
            break;
        }
        fprintf(stdout, "select() got %zd\n", num_ready);
        char* read_buf = NULL;
        char* write_buf = NULL;
//        if (FD_ISSET(accept_sockfd, &readfds)) {
            fprintf(stdout, "accept_sockfd ready for reading\n");
            size_t read_buf_size = 1024;
            size_t write_buf_size = read_buf_size;
            alloc_read_write_buf(read_buf, write_buf, read_buf_size);

            ssize_t nread = 0;
            nread = read_fd(accept_sockfd, read_buf, read_buf_size);
            fprintf(stdout, "accept_sockfd nread: %zd\n", nread);

            if (nread > 1 && strtok_read(read_buf)) {

                const char* rcvd_ok = "HTTP/1.1 200 OK %s\n"
                                      "Content-Length: %d\n"
                                      "Content-Type: text/html; charset=utf-8\n";
                //    Difficult to guarantee safety! Use with extreme care!!! Use
                //    snprpitf!
                snprintf(write_buf, write_buf_size - 1, rcvd_ok, read_buf,
                    strlen(data));

                if (FD_ISSET(accept_sockfd, &writefds)) {
                    fprintf(stdout, "ready for writing\n");
                    if (write(accept_sockfd, write_buf, (strlen(rcvd_ok) + nread)) > 0) {
                        fprintf(stdout, rcvd_ok, read_buf, strlen(data));
                        fflush(stdout);
                    }
                    if (write(accept_sockfd, data, strlen(data)) > 0) {
                        fprintf(stdout, "%s", data);
                        fflush(stdout);
                    }
                }
            }
//        }
        // Sleep before close
        nanosleep(&tm, NULL);
        close(accept_sockfd);
        free(write_buf);
        free(read_buf);
    }

    return ret;
}

#ifdef __linux__

size_t write_epoll_fd(struct epoll_event* ev)
{
    if (ev->events == EPOLLIN) {

        size_t read_buf_size = 1024;
        char* read_buf = (char*)malloc(read_buf_size + 1);
        if (read_buf == NULL) {
            fprintf(stderr, "write_epoll_fd() read_buf malloc error\n");
            return 1;
        }
        memset(read_buf, 0, read_buf_size + 1);
        size_t nread = 0;
        // do {
        nread = read_fd(ev->data.fd, read_buf, read_buf_size);

        size_t write_buf_size = 1024;
        char* write_buf = (char*)malloc(write_buf_size + 1);
        if (write_buf == NULL) {
            fprintf(stderr, "write_epoll_fd() write_buf malloc error\n");
            free(read_buf);
            return 1;
        }

        const char* rcvd_ok = "Apache 2.2.222 received OK %s\n";
        memset(write_buf, 0, write_buf_size + 1);
        snprintf(write_buf, write_buf_size - 1, rcvd_ok, read_buf);

        if (write(ev->data.fd, write_buf, (strlen(rcvd_ok) + nread)) > 0) {
            fprintf(stdout, rcvd_ok, read_buf);
            fflush(stdout);
        }

        free(write_buf);

        // 	read_buf_size -= nread;
        // 	read_buf += nread;
        // } while ((read_buf_size - 1) > 0);

        free(read_buf);
        return 0;
    }
    fprintf(stderr, "write_epoll_fd() %d\n", ev->events);
    return 1;
}
#endif

size_t do_network(config_t* config, size_t epoll_off)
{
    size_t ret = 0;

    char* css_str = NULL;
    char* html_str = NULL;

    if (strlen(config->css) > 0) {
        css_str = read_file(config->css);
        if (css_str != NULL) {
            fprintf(stdout, "serving css:\n%s\n", css_str);
            //      free(css_str);
        }
    }
    if (strlen(config->html) > 0) {
        html_str = read_file(config->html);
        if (html_str != NULL) {
            fprintf(stdout, "serving html:\n%s\n", html_str);
            //      free(html_str);
        }
    }

    // This is pointer to the strncat destination array, which should contain a C
    // string and should be large enough to contain the concatenated resulting
    // string which includes the additional null-character.
    size_t csshtmlsize = strlen(css_str) + strlen(html_str) + 2;
    char* csshtml = malloc(csshtmlsize);
    assert(csshtml);
    memset(csshtml, 0, csshtmlsize);
    strncpy(csshtml, css_str, strlen(css_str));
    strncat(csshtml, html_str, strlen(html_str));
    fprintf(stdout, "csshtml\n%s\n", csshtml);

    size_t sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (set_sock_opts(sockfd, 1)) {
        fprintf(stderr, "set_sock_opts(listen)\n");
        ret = 1;
        return ret;
    }

    struct sockaddr_in my_addr;
    memset(&my_addr, 0, sizeof(struct sockaddr_in));

    my_addr.sin_family = AF_INET;
    size_t port_strlen = strlen(config->port);
    if (port_strlen == 0) {
        fprintf(stderr, "strlen(port) == 0");
        ret = 1;
        return ret;
    }

    fprintf(stdout, "strlen(port) == %zu\n", port_strlen);
    fprintf(stdout, "port %s\n", config->port);
    my_addr.sin_port = htons(atoi(config->port));

    if (inet_pton(AF_INET, config->address, &my_addr.sin_addr.s_addr) != 1) {
        fprintf(stderr, "inet_pton(%s) %s\n", config->address, strerror(errno));
        ret = 1;
        return ret;
    }

    socklen_t sockaddr_size = sizeof(my_addr);
    if (bind(sockfd, (struct sockaddr*)&my_addr, sockaddr_size)) {
        fprintf(stderr, "bind(%zu) %s\n", sockfd, strerror(errno));
        ret = 1;
        return ret;
    }

    size_t backlog = 1024;
    if (listen(sockfd, backlog) == 1) {
        fprintf(stderr, "listen(%zu) %s\n", sockfd, strerror(errno));
    }

#ifndef __linux__
    if (epoll_off) {
        fprintf(stdout, "epoll_off set - don't have epoll on current OS\n");
    }

    if (write_no_epoll_fd(sockfd, (struct sockaddr*)&my_addr, sockaddr_size,
            csshtml)) {
        ret = 1;
        fprintf(stderr, "write_no_epoll_fd(%zu) 1\n", sockfd);
    }
#else
    if (epoll_off) {
        fprintf(stdout, "epoll_off set\n");

        if (write_no_epoll_fd(sockfd, (struct sockaddr*)&my_addr, sockaddr_size,
                csshtml)) {
            ret = 1;
            fprintf(stderr, "write_no_epoll_fd(%zu) 1\n", sockfd);
        }

    } else {
        ssize_t epfd = 0;
        if ((epfd = epoll_create(1)) == -1) {
            fprintf(stderr, "epoll_create() -1 %s\n", strerror(errno));
            ret = 1;
            return ret;
        }
        fprintf(stdout, "epoll_create() %zi\n", epfd);

        struct epoll_event event = {
            .events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET, .data.fd = sockfd
        };
        if ((epoll_ctl(epfd, EPOLL_CTL_ADD, event.data.fd, &event)) == -1) {
            fprintf(stderr, "epoll_ctl() -1 %s\n", strerror(errno));
            ret = 1;
            return ret;
        }

        size_t epoll_evts = 1;
        struct epoll_event ev, events[epoll_evts];
        memset(&ev, 0, sizeof(struct epoll_event));
        memset(&events, 0, epoll_evts * sizeof(struct epoll_event));

        ssize_t numfds = 0;
        for (;;) { // Loop
            if ((numfds = epoll_wait(epfd, events, epoll_evts, -1)) == -1 && (errno != EINTR)) {
                fprintf(stderr, "epoll_wait() -1 %s\n", strerror(errno));
                ret = 1;
                break;
            }

            for (ssize_t i = 0; i < numfds; ++i) {

                if (events[i].events & (EPOLLERR | EPOLLHUP)) {
                    fprintf(stderr, "epoll fd %d got (EPOLLERR | EPOLLHUP)\n",
                        events[i].data.fd);
                    close(events[i].data.fd);
                    continue;
                }

                if (events[i].data.fd == (int)sockfd) {
                    fprintf(stdout, "events got listen sockfd\n");
                    ssize_t accept_sockfd = 0;
                    if ((accept_sockfd = accept(sockfd, (struct sockaddr*)&my_addr,
                             &sockaddr_size))
                        == -1) {
                        fprintf(stderr, "accept() -1 %s\n", strerror(errno));
                        continue;
                    }
                    if (set_sock_opts(accept_sockfd, 0)) {
                        fprintf(stderr, "fcntl(accept) -1 %s\n", strerror(errno));
                        ret = 1;
                        break;
                    }
                    ev.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
                    ev.data.fd = accept_sockfd;
                    ssize_t epoll_ret = 0;
                    if ((epoll_ret = epoll_ctl(epfd, EPOLL_CTL_ADD, ev.data.fd, &ev)) == -1) {
                        fprintf(stderr, "accept epoll_ctl() -1 %s\n", strerror(errno));
                        ret = 1;
                        break;
                    }
                    epoll_evts++;

                } else {

                    if (write_epoll_fd(&events[i])) {

                        fprintf(stderr, "closing %d\n", events[i].data.fd);
                        close(events[i].data.fd);
                        epoll_evts--;
                    }
                    continue;
                }
            }
        }
        close(epfd);
    }
#endif

    free(csshtml);
    free(html_str);
    free(css_str);
    close(sockfd);
    return ret;
}
