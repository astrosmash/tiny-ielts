#include "main.h"

int main(int argc, char **argv) {
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
      fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n", *argv);
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
    config_t *config_str = read_config(config);

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
