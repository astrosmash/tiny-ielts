/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

static uint8_t test1(int num, ...) {
    va_list test1_list;
    va_start(test1_list, num);
    char *args = NULL;
    args = (char *) malloc(sizeof (char) * num);
    assert(args);

    for (uint8_t l = 0; l < num; ++l) {
        *(args + l) = va_arg(test1_list, int);

    }
    va_end(test1_list);

    fprintf(stdout, "test1 %s\n", args);
    return EXIT_SUCCESS;
}

static uint8_t test2(int num, ...) {
    uint8_t num_processed = 0;
    va_list test2_list;
    va_start(test2_list, num);

    while (num_processed < num) {
        uint8_t my_num = va_arg(test2_list, int);
        fprintf(stdout, "test2 %u\n", my_num);
        ++num_processed;
    }
    va_end(test2_list);

    //    fprintf(stdout, "test2 %u\n", num_processed);
    return EXIT_SUCCESS;

}

int main(void) {
    test1(2, 'z', 'n');

    fprintf(stdout, "va_args \n");

    uint8_t res = test2(4, 2, 1, 34, 66);
    return res;
}