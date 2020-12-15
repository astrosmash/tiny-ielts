#include "userspace.h"
static char* read_file(const char*);
static config_t* allocate_config(config_t*);
static void free_config(config_t*);
static config_t* read_config(const char*);
static size_t do_network(config_t*, size_t);


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
	size_t buf_size = 1024; // FIXME: will overflow on large file
	config->port = (char*)malloc(sizeof(char));
	config->address = (char*)malloc(sizeof(char));
	config->css = (char*)malloc(buf_size);
	config->html = (char*)malloc(buf_size);

	if (config->port == NULL || config->address == NULL || config->css == NULL || config->html == NULL) {
		fprintf(stderr, "allocate_config malloc error\n");
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

	char* line = strtok(file, "\n");
	while (line != NULL)
	{
		if (sscanf(line, "address = %s\n", config->address) == 1) {
			fprintf(stdout, "scanned address %s\n", config->address);
		}
		if (sscanf(line, "port = %s\n", config->port) == 1) {
			fprintf(stdout, "scanned port %s\n", config->port);
		}
		if (sscanf(line, "css = %s\n", config->css) == 1) {
			fprintf(stdout, "scanned css %s\n", config->css);
		}
		if (sscanf(line, "html = %s\n", config->html) == 1) {
			fprintf(stdout, "scanned html %s\n", config->html);
		}
		line = strtok(NULL, "\n");
	}

	fprintf(stdout, "read_config address: %s port: %s css: %s html: %s\n", config->address, config->port, config->css, config->html);
	free(file);
	return config;
}


static size_t do_network(config_t* config, size_t epoll_off) {
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
	if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1) {
		fprintf(stderr, "fcntl() -1 %s\n", strerror(errno));
		return 1;
	}
	size_t one = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one)) == -1) {
		fprintf(stderr, "setsockopt() -1 %s\n", strerror(errno));
		return 1;
	}

	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET;
	size_t port_strlen = strlen(config->port);
	if (port_strlen == 0) {
		fprintf(stderr, "strlen(port) == 0");
		return 1;
	}

	fprintf(stdout, "strlen(port) == %zu\n", port_strlen);
	fprintf(stdout, "port %s\n", config->port);
	my_addr.sin_port = htons(atoi(config->port));

	if (inet_pton(AF_INET, config->address, &my_addr.sin_addr.s_addr) != 1) {
		fprintf(stderr, "inet_pton(%s) %s\n", config->address, strerror(errno));
		return 1;
	}

	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr))) {
		fprintf(stderr, "bind(%zu) %s\n", sockfd, strerror(errno));
		return 1;
	}

	size_t backlog = 1024;
	if (listen(sockfd, backlog) == 1) {
		fprintf(stderr, "listen(%zu) %s\n", sockfd, strerror(errno));
	}

	#ifndef __linux__
		if (epoll_off) {
			fprintf(stdout, "epoll_off set - don't have epoll on current OS\n");
		}
		for (;;) {}
	#else
		if (epoll_off) {
			fprintf(stdout, "epoll_off set\n");

			for (;;) {}

		} else {
			size_t epfd = 0;
			if ((epfd = epoll_create(1)) == -1) {
				fprintf(stderr, "epoll_create() -1 %s\n", strerror(errno));
				return 1;
			}
			fprintf(stdout, "epoll_create() %zu\n", epfd);

			struct epoll_event event = { .events = EPOLLIN, .data.fd = sockfd };
			size_t ret = 0;
			if ((ret = epoll_ctl(epfd, EPOLL_CTL_ADD, event.data.fd, &event)) == -1) {
				fprintf(stderr, "epoll_ctl() -1 %s\n", strerror(errno));
				return 1;
			}
			fprintf(stdout, "epoll_ctl() %zu\n", ret);

			for (;;) {}

			close(epfd);
		}
	#endif

	close(sockfd);
	return 0;
}



int main(int argc, char **argv)
{
    char *config = NULL;
    size_t debug = 0, epoll_off = 0, opt = 0;
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

			if (config_str) {
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
