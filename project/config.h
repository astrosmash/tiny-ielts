typedef struct __attribute__((aligned)) {
  char *port;
  char *address;
  char *css;
  char *html;
} config_t;

extern char *read_file(const char *);
extern config_t *allocate_config(config_t *);
extern void free_config(config_t *);
extern config_t *read_config(const char *);

#include "config.c"
