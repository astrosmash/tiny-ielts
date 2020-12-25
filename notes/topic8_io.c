#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

int main(int argc, char** argv) {

    struct timespec ts = {.tv_sec = 1};
    size_t nsl_res;
    if ((nsl_res = nanosleep(&ts, NULL) == -1)) {
        fprintf(stderr, "nanosleep(%ld) %s\n", ts.tv_nsec, strerror(errno));
        return EXIT_FAILURE;
    }

    //    assert(argc == 2);
    uint8_t scanbuf = 10;
    char fname[scanbuf + 1];
    fprintf(stderr, "Enter filename\n");
    if (scanf("%10s", fname) != 1) {
        fprintf(stderr, "%s %s\n", fname, strerror(errno));
        return EXIT_FAILURE;
    }

    const char* fmode = "r";
    FILE* file = NULL;
    if ((file = fopen(fname, fmode)) == NULL) {
        fprintf(stderr, "fopen(%s) %s\n", fname, strerror(errno));
        return EXIT_FAILURE;
    }

    size_t buf_size = 1024;
    char* buf = (char*) malloc(buf_size);
    if (buf == NULL) {
        fprintf(stderr, "read_file malloc error\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    size_t ret = fread(buf, sizeof (char), buf_size, file);
    fclose(file);
    fprintf(stdout, "fread(%s) %zu bytes %s\n", fname, ret, buf);

    if (ret) {
        // Number of triangles is the first digit.
        size_t num_tr = 0;

        // Scan first digit - this will be total # of triangles
        if (sscanf(buf, "%zu", &num_tr) == 1) {
            fprintf(stdout, "Scanned %zu elements\n", num_tr);
        }

        fprintf(stdout, "Buf populated\n");
        char testchar;

        do {
            testchar = getchar();
            if (testchar == EOF) {
                fprintf(stderr, "getc %s\n", strerror(errno));
                break;
            }
        } while (testchar == '\n');
        fprintf(stdout, "Got char %c\n", testchar);
    }

    free(buf);
    return EXIT_SUCCESS;
}
