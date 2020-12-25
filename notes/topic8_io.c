#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

int main(int argc, char** argv) {

//    struct timespec ts = {.tv_sec = 1};
//    size_t nsl_res;
//    if ((nsl_res = nanosleep(&ts, NULL) == -1)) {
//        fprintf(stderr, "nanosleep(%ld) %s\n", ts.tv_nsec, strerror(errno));
//        return EXIT_FAILURE;
//    }
//
//    //    assert(argc == 2);
//    uint8_t scanbuf = 10;
//    char fname[scanbuf + 1];
//    fprintf(stderr, "Enter filename\n");
//    if (scanf("%10s", fname) != 1) {
//        fprintf(stderr, "%s %s\n", fname, strerror(errno));
//        return EXIT_FAILURE;
//    }
//
//    const char* fmode = "r";
//    FILE* file = NULL;
//    if ((file = fopen(fname, fmode)) == NULL) {
//        fprintf(stderr, "fopen(%s) %s\n", fname, strerror(errno));
//        return EXIT_FAILURE;
//    }
//
//    size_t buf_size = 1024;
//    char* buf = (char*) malloc(buf_size);
//    if (buf == NULL) {
//        fprintf(stderr, "read_file malloc error\n");
//        fclose(file);
//        return EXIT_FAILURE;
//    }
//
//    size_t ret = fread(buf, sizeof (char), buf_size, file);
//    fclose(file);
//    fprintf(stdout, "fread(%s) %zu bytes %s\n", fname, ret, buf);
//
//    if (ret) {
//        // Number of triangles is the first digit.
//        size_t num_tr = 0;
//
//        // Scan first digit - this will be total # of triangles
//        if (sscanf(buf, "%zu", &num_tr) == 1) {
//            fprintf(stdout, "Scanned %zu elements\n", num_tr);
//        }
//
//        fprintf(stdout, "Buf populated\n");
//        char testchar;
//
//        do {
//            testchar = getchar();
//            if (testchar == EOF) {
//                fprintf(stderr, "getc %s\n", strerror(errno));
//                break;
//            }
//        } while (testchar == '\n');
//
//
//        if (testchar != putchar(testchar)) {
//            fprintf(stderr, "putchar %s\n", strerror(errno));
//            return EXIT_FAILURE;
//        }
//        fprintf(stdout, "Got char %c\n", testchar);
//        }
//    }
//
//    free(buf);

    const char* fname = "/tmp/input";
    const char* fmode = "r";
    FILE* file = NULL;
    if ((file = fopen(fname, fmode)) == NULL) {
        fprintf(stderr, "fopen(%s) %s\n", fname, strerror(errno));
        return EXIT_FAILURE;
    }

    size_t buf_size = 1024;
    char* buf = (char*) malloc(buf_size);
    if (buf == NULL) {
        fprintf(stderr, "malloc %s\n", strerror(errno));
        fclose(file);
        return EXIT_FAILURE;
    }

    // added \0 (but it's already size-1)
    if (fgets(buf, buf_size - 1, file) == NULL) {
        fprintf(stderr, "fgets %s\n", strerror(errno));
        fclose(file);
        return EXIT_FAILURE;
    }
    fprintf(stdout, "%s", buf);

    uint16_t bsize = 512;
    const char *snpr = "This is snprintf to join with %s";
    char snprbuf[bsize];
    assert(snprbuf);
    snprintf(snprbuf, bsize - 1, snpr, buf);
    fprintf(stdout, "snprbuf: %s", snprbuf); // \n is added by snprintf

    size_t dd;
    size_t fscanf_res = 0;
    // file is already on next line b/c of fgets
    if ((fscanf_res = fscanf(file, "%zd", &dd) == 1)) {
        fprintf(stdout, "dd: %zu\n", dd); // \n is added
    }

    fclose(file);

    return EXIT_SUCCESS;
}
