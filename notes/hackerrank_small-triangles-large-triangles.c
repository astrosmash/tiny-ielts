// Task code -------------------------------------------------------------------
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

struct triangle {
    int a;
    int b;
    int c;
};

typedef struct triangle triangle;
// Task code ---------------------------------------------------------------------


typedef enum { CLASSIC_BUBBLE = 1,
    MY = 2 } method;
typedef enum { SQRT = 1,
    SUM = 2 } area_type;

#include <assert.h>
#include <stdint.h>
#include <string.h>

void sort_by_area(triangle* tr, int n, method m, area_type at)
{
    /**
    * Sort an array a of the length n
    */
    assert(tr);
    assert(1 <= n && n <= 100);

    triangle result[n];
    unsigned int weight[n];

    // Populate
    for (int i = 0; i < n; ++i) {
        fprintf(stdout, "iteration #%u\n", i);
        assert(tr + i);
        triangle* ctr = tr + i;

        assert(1 <= ctr->a && 1 <= ctr->b && 1 <= ctr->c);
        assert(70 >= ctr->a && 70 >= ctr->b && 70 >= ctr->c);
        assert(ctr->a + ctr->b > ctr->c && ctr->a + ctr->c > ctr->b && ctr->b + ctr->c > ctr->a);
        // fprintf(stdout, "asserted ok. %u %u %u\n", ctr->a, ctr->b, ctr->c);

        const unsigned int hot = (ctr->a + ctr->b + ctr->c) / 2;
        const double sqrtable = hot * (hot - ctr->a) * (hot - ctr->b) * (hot - ctr->c);
        const unsigned int area = (at == SQRT) ? sqrt(sqrtable) : ctr->a + ctr->b + ctr->c;

        result[i] = *ctr;
        weight[i] = area;

        // On old (my) method, we sort inside population loop
        if (m == MY) {
            fprintf(stdout, "method: MY\n");

            // AND sort
            for (int j = 0; j < i; ++j) {
                if (weight[j] > weight[i]) {
                    triangle temp = result[j];
                    // fprintf(stdout, "temp permut. %u %u %u %u\n", j, temp.a, temp.b, temp.c);

                    unsigned int tempw = weight[j];
                    // fprintf(stdout, "PREV permut. %u(%i) < %u(%i)\n", weight[i], i, weight[j], j);

                    result[j] = result[i];
                    weight[j] = weight[i];
                    result[i] = temp;
                    weight[i] = tempw;
                    // fprintf(stdout, "permut. %i %i %i %i\n", j, result[j].a, result[j].b, result[j].c);
                }
            }
        }
    }

    if (m == CLASSIC_BUBBLE) {
        fprintf(stdout, "method: CLASSIC_BUBBLE\n");

        // Classic bubblesort
        for (int i = 0; i < n - 1; ++i) {
            for (int j = 0; j < n - i - 1; ++j) {
                if (weight[j] > weight[j + 1]) {
                    triangle temp = result[j];
                    // fprintf(stdout, "temp permut. %u %u %u %u\n", j, temp.a, temp.b, temp.c);

                    unsigned int tempw = weight[j];
                    // fprintf(stdout, "PREV permut. %u(%u) < %u(%u)\n", weight[i], i, weight[j], j);

                    result[j] = result[j + 1];
                    weight[j] = weight[j + 1];
                    result[j + 1] = temp;
                    weight[j + 1] = tempw;
                    // fprintf(stdout, "permut. %i %i %i %i\n", j, result[j].a, result[j].b, result[j].c);
                }
            }
        }
    }

    memcpy(tr, result, n * sizeof(triangle));
}

int main(int argc, char** argv)
{
    assert(argc == 3);

    const char* one = "1";
    const char* two = "2";
    uint8_t algo = 0;
    uint8_t at = 0;
    int8_t strcmp_res = 99;

    if ((strcmp_res = strcmp(one, argv[1]) == 0)) {
        fprintf(stdout, "activating algo CLASSIC_BUBBLE \n");
        algo = 1;
    } else if ((strcmp_res = strcmp(two, argv[1]) == 0)) {
        fprintf(stdout, "activating algo MY \n");
        algo = 2;
    } else {
        fprintf(stderr, "Algo (%s) should be 1 (CLASSIC_BUBBLE) or 2 (MY)\n", argv[1]);
        return EXIT_FAILURE;
    }

    if ((strcmp_res = strcmp(one, argv[2]) == 0)) {
        fprintf(stdout, "activating area_type SQRT \n");
        at = 1;
    } else if ((strcmp_res = strcmp(two, argv[2]) == 0)) {
        fprintf(stdout, "activating area_type SUM \n");
        at = 2;
    } else {
        fprintf(stderr, "area_type (%s) should be 1 (SQRT) or 2 (SUM)\n", argv[2]);
        return EXIT_FAILURE;
    }

    const char* fname = "/tmp/input";
    const char* fmode = "r";
    FILE* file = NULL;

    if ((file = fopen(fname, fmode)) == NULL) {
        fprintf(stderr, "fopen(%s) no file\n", fname);
        return EXIT_FAILURE;
    }

    size_t buf_size = 1024; // FIXME: will overflow on large file
    char* buf = (char*)malloc(buf_size);
    if (buf == NULL) {
        fprintf(stderr, "read_file malloc error\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    size_t ret = fread(buf, sizeof(char), buf_size, file);
    fclose(file);
    fprintf(stdout, "fread(%s) %zu bytes %s\n", fname, ret, buf);

    if (ret) {
        // Number of triangles is the first digit.
        size_t num_tr = 0;

        // Scan first digit - this will be total # of triangles
        if (sscanf(buf, "%zu", &num_tr) == 1) {
            fprintf(stdout, "Scanned %zu elements\n", num_tr);
        }

        triangle* trg = malloc(num_tr * sizeof(triangle));
        assert(trg);
        // Garbage found here otherwise
        memset(trg, 0, num_tr * sizeof(triangle));

        char* strtok_saveptr = NULL;
        char* line = strtok_r(buf, "\n", &strtok_saveptr);
        size_t lnn = 0;

        while (line != NULL) {
            // fprintf(stdout, "Iteration #%zu got line %s\n", lnn, line);

            if (sscanf(line, "%d %*d %*d\n", &(trg + lnn)->a) == 1) {
                // fprintf(stdout, "#%zu scanned a %u\n", lnn, (trg + lnn)->a);
            }
            if (sscanf(line, "%*d %d %*d\n", &(trg + lnn)->b) == 1) {
                // fprintf(stdout, "#%zu scanned b %u\n", lnn, (trg + lnn)->b);
            }
            if (sscanf(line, "%*d %*d %d\n", &trg[lnn].c) == 1) {
                // fprintf(stdout, "#%zu scanned c %u\n", lnn, trg[lnn].c);
            }
            line = strtok_r(NULL, "\n", &strtok_saveptr);

            // Populate only if 3 sides were scanned.
            if ((trg + lnn)->a > 0 && (trg + lnn)->b > 0 && (trg + lnn)->c > 0) {
                ++lnn;
            }
        }
        fprintf(stdout, "Buf populated\n");
        fprintf(stdout, "\n");

        sort_by_area(trg, num_tr, algo, at);
        for (uint8_t i = 0; i < num_tr; i++) {
            printf("%d %d %d\n", trg[i].a, trg[i].b, trg[i].c);
        }
        free(trg);
    }

    free(buf);
    return EXIT_SUCCESS;
}
