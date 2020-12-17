#include "userspace.h"
static char* read_file(const char*);
static config_t* allocate_config(config_t*);
static void free_config(config_t*);
static config_t* read_config(const char*);
static size_t do_network(config_t*, size_t);
static size_t set_sock_opts(size_t);
static size_t read_from_socket(size_t, char*, size_t);
static size_t write_select_fd(size_t);

#ifdef __linux__
static size_t write_epoll_fd(struct epoll_event*);
#endif


static char* read_file(const char* filename) {
	fprintf(stdout, "read_file(%s)\n", filename);
	const char* mode = "r";
	FILE* file = NULL;

	if ((file = fopen(filename, mode)) == NULL) {
			fprintf(stderr, "fopen(%s) no file\n", filename);
	 		return NULL;
	}

	size_t buf_size = 1024; // FIXME: will overflow on large file
	char* buf = (char*)malloc(buf_size);
	if (buf == NULL) {
		fprintf(stderr, "read_file malloc error\n");
		fclose(file);
		return NULL;
	}

	size_t ret = fread(buf, sizeof(char), buf_size, file);
	fclose(file);
	fprintf(stdout, "fread(%s) %zu bytes\n", filename, ret);
	return buf;
}


static config_t* allocate_config(config_t* config) {
	config = malloc(sizeof(config_t));
	config->port = (char*)malloc(sizeof(char));
	config->address = (char*)malloc(sizeof(char));
	config->css = (char*)malloc(sizeof(char));
	config->html = (char*)malloc(sizeof(char));

	if (config->port == NULL || config->address == NULL || config->css == NULL || config->html == NULL) {
		fprintf(stderr, "allocate_config malloc error\n");
		free(config);
		return NULL;
	}
	return config;
}


static void free_config(config_t* config) {
	free(config->address);
	free(config->port);
	free(config->css);
	free(config->html);
	free(config);
	fprintf(stderr, "free_config done\n");
}


static config_t* read_config(const char* filename) {
	fprintf(stdout, "read_config(%s)\n", filename);
	char* file = NULL;

	if ((file = read_file(filename)) == NULL) {
			fprintf(stderr, "read_file(%s) error\n", filename);
	 		return NULL;
	}

	config_t* config = NULL;
	if ((config = allocate_config(config)) == NULL) {
		return NULL;
	}

	char* strtok_saveptr = NULL;
	char* line = strtok_r(file, "\n", &strtok_saveptr);
	while (line != NULL)
	{
		if (sscanf(line, "address = %14s\n", config->address) == 1) {
			fprintf(stdout, "scanned address %s\n", config->address);
		}
		if (sscanf(line, "port = %4s\n", config->port) == 1) {
			fprintf(stdout, "scanned port %s\n", config->port);
		}
		if (sscanf(line, "css = %63s\n", config->css) == 1) {
			fprintf(stdout, "scanned css %s\n", config->css);
		}
		if (sscanf(line, "html = %63s\n", config->html) == 1) {
			fprintf(stdout, "scanned html %s\n", config->html);
		}
		line = strtok_r(NULL, "\n", &strtok_saveptr);
	}

	fprintf(stdout, "read_config address: %s port: %s css: %s html: %s\n", config->address, config->port, config->css, config->html);
	free(file);
	return config;
}


static size_t set_sock_opts(size_t sockfd) {
	if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1) {
		fprintf(stderr, "fcntl() -1 %s\n", strerror(errno));
		return 1;
	}
	size_t one = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
		fprintf(stderr, "setsockopt() -1 %s\n", strerror(errno));
		return 1;
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one)) == -1) {
		fprintf(stderr, "setsockopt() -1 %s\n", strerror(errno));
		return 1;
	}
	return 0;
}


static size_t read_from_socket(size_t sockfd, char* read_buf, size_t read_buf_size) {
	ssize_t nread = 0;
	nread = read(sockfd, read_buf, read_buf_size);

	if (nread < 0 && (errno == EINTR || errno == EWOULDBLOCK)) {
		return 0;
	}

	if (nread < 0) {
		fprintf(stderr, "write_epoll_fd() read() %s\n", strerror(errno));
		return 1;
	}

	if (nread == 0) {
		fprintf(stdout, "write_epoll_fd() read() closed socket\n");
		return 0;
	}

	fprintf(stdout, "write_epoll_fd() read_buf() %s\n", read_buf);
	fflush(stdout);
	return nread;
}


static size_t write_select_fd(size_t sockfd) {
	fd_set rfds = {0};
	struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };

	FD_ZERO(&rfds);
	FD_SET(sockfd, &rfds); // TODO: rfds empty - select(1, [], NULL, NULL, {tv_sec=5, tv_usec=0}) 

	ssize_t select_ret = 0;
	if ((select_ret = select(1, &rfds, NULL, NULL, &tv)) == -1) {
		fprintf(stderr, "write_select_fd(%zu) select -1 %s\n", sockfd, strerror(errno));
		return 1;
	}

	if (select_ret == 0) {
		fprintf(stdout, "write_select_fd(%zu) no data within 5s\n", sockfd);
		return 0;
	}

	fprintf(stdout, "write_select_fd(%zu) select %zd\n", sockfd, select_ret);

	size_t read_buf_size = 1024;
	char* read_buf = (char*)malloc(read_buf_size + 1);
	if (read_buf == NULL) {
		fprintf(stderr, "write_epoll_fd() read_buf malloc error\n");
		return 1;
	}
	size_t nread = 0;
	nread = read_from_socket(sockfd, read_buf, read_buf_size);

	size_t write_buf_size = 1024;
	char* write_buf = (char*)malloc(write_buf_size + 1);
	if (write_buf == NULL) {
		fprintf(stderr, "write_epoll_fd() write_buf malloc error\n");
		free(read_buf);
		return 1;
	}

	static const char* rcvd_ok = "Apache 2.2.222 received OK %s\n";
	sprintf(write_buf, rcvd_ok, read_buf);

	if (write(sockfd, write_buf, (strlen(rcvd_ok) + nread)) > 0) {
		fprintf(stdout, rcvd_ok, read_buf);
		fflush(stdout);
	}

	free(write_buf);
	free(read_buf);

	return 0;
}


#ifdef __linux__
static size_t write_epoll_fd(struct epoll_event* ev) {
	if (ev->events == EPOLLIN) {

		size_t read_buf_size = 1024;
		char* read_buf = (char*)malloc(read_buf_size + 1);
		if (read_buf == NULL) {
			fprintf(stderr, "write_epoll_fd() read_buf malloc error\n");
			return 1;
		}
		size_t nread = 0;
		// do {
			nread = read_from_socket(ev->data.fd, read_buf, read_buf_size);

			size_t write_buf_size = 1024;
			char* write_buf = (char*)malloc(write_buf_size + 1);
			if (write_buf == NULL) {
 				fprintf(stderr, "write_epoll_fd() write_buf malloc error\n");
				free(read_buf);
				return 1;
			}

			static const char* rcvd_ok = "Apache 2.2.222 received OK %s\n";
			sprintf(write_buf, rcvd_ok, read_buf);

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


static size_t do_network(config_t* config, size_t epoll_off) {
	size_t ret = 0;

	if (strlen(config->css) > 0) {
		char* css_str = read_file(config->css);
		if (css_str != NULL) {
			fprintf(stdout, "serving css:\n%s\n", css_str);
			free(css_str);
		}
	}
	if (strlen(config->html) > 0) {
		char* html_str = read_file(config->html);
		if (html_str != NULL) {
			fprintf(stdout, "serving html:\n%s\n", html_str);
			free(html_str);
		}
	}

	size_t sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (set_sock_opts(sockfd)) {
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
	if (bind(sockfd, (struct sockaddr *) &my_addr, sockaddr_size)) {
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

		if (write_select_fd(sockfd)) {
			ret = 1;
			fprintf(stderr, "write_select_fd(%zu) 1\n", sockfd);
		}
	#else
		if (epoll_off) {
			fprintf(stdout, "epoll_off set\n");

			if (write_select_fd(sockfd)) {
				ret = 1;
				fprintf(stderr, "write_select_fd(%zu) 1\n", sockfd);
			}

		} else {
			ssize_t epfd = 0;
			if ((epfd = epoll_create(1)) == -1) {
				fprintf(stderr, "epoll_create() -1 %s\n", strerror(errno));
				ret = 1;
				return ret;
			}
			fprintf(stdout, "epoll_create() %zu\n", epfd);

			struct epoll_event event = { .events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET, .data.fd = sockfd };
			if ((epoll_ctl(epfd, EPOLL_CTL_ADD, event.data.fd, &event)) == -1) {
				fprintf(stderr, "epoll_ctl() -1 %s\n", strerror(errno));
				ret = 1;
				return ret;
			}

			size_t epoll_evts = 1;
			struct epoll_event ev, events[epoll_evts];
			memset(&ev, 0, sizeof(struct epoll_event));
			memset(&events, 0, sizeof(events));

			ssize_t numfds = 0;
			for (;;) {
				if ((numfds = epoll_wait(epfd, events, epoll_evts, -1)) == -1 && (errno != EINTR)) {
					fprintf(stderr, "epoll_wait() -1 %s\n", strerror(errno));
					ret = 1;
					break;
				}

				for (ssize_t i = 0; i < numfds; ++i) {

					if (events[i].events & (EPOLLERR | EPOLLHUP)) {
						fprintf(stderr, "epoll fd %d got (EPOLLERR | EPOLLHUP)\n", events[i].data.fd);
						close(events[i].data.fd);
						continue;
					}

					if (events[i].data.fd == (int)sockfd) {
						fprintf(stdout, "events got listen sockfd\n");
						ssize_t accept_sockfd = 0;
						if ((accept_sockfd = accept(sockfd, (struct sockaddr *) &my_addr, &sockaddr_size)) == -1) {
							fprintf(stderr, "accept() -1 %s\n", strerror(errno));
							continue;
						}
						if (fcntl(accept_sockfd, F_SETFL, fcntl(accept_sockfd, F_GETFD, 0)|O_NONBLOCK) == -1) {
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

	close(sockfd);
	return ret;
}



int main(int argc, char **argv)
{
    char *config = NULL;
    ssize_t opt = 0;
    size_t debug = 0, epoll_off = 0;
    size_t nsecs = 0, tfnd = 0;

    while ((opt = getopt(argc, argv, "nc:t:de")) != -1) {
        switch (opt) {
        case 'd':
            debug = 1;
            break;
        case 'e':
            epoll_off = 1;
            break;
        case 'c':
            config = optarg;
            break;
        case 't':
            nsecs = atoi(optarg);
            tfnd = 1;
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n",
                    *argv);
            exit(EXIT_FAILURE);
        }
    }

    // if (optind >= argc) {
    //      fprintf(stderr, "Expected argument after options\n");
    //      exit(EXIT_FAILURE);
    // }
		//
    // if (*(argv+1) != NULL) {
		// 	fprintf(stdout, "argc=%d; argv=%s\n", argc, *(argv+1));
    // }
		//
    // for (size_t i = 0; i <= strlen(*argv); ++i) {
		// 	fprintf(stdout, "argv[%zu]=%c\n", i, *(*argv+i));
    // }
		//
    // fprintf(stdout, "debug=%zu; tfnd=%zu; optind=%d\n", debug, tfnd, optind);

		if (config != NULL) {
			fprintf(stdout, "config=%s\n", config);
			config_t* config_str = read_config(config);

			if (config_str != NULL) {
				if (do_network(config_str, epoll_off)) {
					fprintf(stderr, "do_network() error\n");
				}
				free_config(config_str);
			}
    }

		fprintf(stdout, "name argument = %s\n", *(argv + optind));

    /* Other code omitted */
    exit(EXIT_SUCCESS);
}
