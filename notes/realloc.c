#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct mystr {
    size_t a;
    size_t b;
    uint64_t c;
} mystr_t;

int main(void) {
    mystr_t *str_new = (mystr_t *) malloc(sizeof (mystr_t));
    assert(str_new);
    fprintf(stdout, "Malloced %p\n", str_new);

    mystr_t *str_new_realloc = (mystr_t *) realloc(str_new, sizeof (mystr_t) + 64);
    assert(str_new_realloc);
    fprintf(stdout, "realloc %p\n", str_new_realloc);

    if (str_new == str_new_realloc) {
        fprintf(stdout, "realloced on same base address! %p\n", str_new);
        free(str_new);
        return (EXIT_SUCCESS);
    }

    free(str_new_realloc);
    free(str_new);
    return (EXIT_SUCCESS);
}