/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

//// Funcptr is a ptr to function that returns int and reads no args.
//typedef int (*funcptr)();
//typedef funcptr (*funcptr77)();
//// Ptrfuncptr is a ptr to function that returns funcptr and reads void.
//typedef funcptr77(*ptrfuncptr)(void);
//
//funcptr pseudostart, start(void), stop();
//funcptr state1(), state2(), state3();
//
//static void statemachine(void) {
//    funcptr77 state = start;
//    while (state != stop) {
//        state = (funcptr77)(*state)();//static void statemachine(void) {

//    }
//}
//
//funcptr start(void) {
//    return (funcptr)state1;
//}

//struct functhunk {
//    struct functhunk(*func)();
//};
//
//struct functhunk start(), stop();
//struct functhunk state1(), state2(), state3;
//
//void statemachine(void) {
//    struct functhunk state = {start};
//    while (state.func != stop) {
//        state = (*state.func)();
//    }
//}
//
//struct functhunk start() {
//    struct functhunk ret = {0};
//    ret.func = state1;
//    return ret;
//};

void infinite_recursion(void) {
    fprintf(stdout, "infinite recursion\n ");
    infinite_recursion();
}

void infinite_recursion2(int st) {
    fprintf(stdout, "infinite recursion%d\n ", st);
    infinite_recursion2(++st);
}
int main(void) {
    uint16_t two_dim_arr_of_2bytes[255][384] = {0};
    two_dim_arr_of_2bytes[69][42] = 4099;
    two_dim_arr_of_2bytes[146][222] = 50;

    // two_dim_arr_of_2bytes[69][42]
    fprintf(stdout, "%u\t\n", *(*(two_dim_arr_of_2bytes + 69) + 42));

    // two_dim_arr_of_2bytes[146][222]
    fprintf(stdout, "%u\t\n", *(*(two_dim_arr_of_2bytes)+(384 * 146) + 222));

    uint8_t onefourtysix = 146;
    onefourtysix[two_dim_arr_of_2bytes][222] = 70;
    // two_dim_arr_of_2bytes[146][222]
    fprintf(stdout, "%u\t\n", *(*(two_dim_arr_of_2bytes)+(384 * 146) + 222));

    //    start();
    uint16_t *testd = (uint16_t *) malloc(255 * 384 * sizeof (uint16_t));
    assert(testd);
    memset(testd, 0, sizeof(*testd));
    *(testd + 254) = 99;
    fprintf(stdout, "QQ %u\t\n", *(testd + 254));
    fprintf(stdout, "QQ %u\t\n", *(testd + 254 * 383));
    *(testd + 1 * 4) = 2;
    fprintf(stdout, "QQ %u\t\n-----\n",  *(testd + 1 * 4));

    uint16_t testd2[255][384] = { { 1, 3 }, { 2, 3, 4 }};

    fprintf(stdout, "QQ %u\t\n", *testd2[0]);
    fprintf(stdout, "BB %u\t\n", *(*(testd2 + 1) + 1)); // testd2[1][1] // OR *(testd2)+384+1
    fprintf(stdout, "ZZ %u\t\n", *(*(testd2 + 1) + 2)); // testd2[1][2] // OR *(testd2)+385+1
    testd2[69][42] = 4099;
    fprintf(stdout, "MM %u\t\n", *(*(testd2 + 69) + 42)); // testd2[69][42]


    testd2[146][222] = 50;
    fprintf(stdout, "RR %u\t\n", *(*(testd2)+(384*146)+222)); // testd2[146][222]
    // You may be wondering how pointers and multidimensional arrays interact.
    // Let's look at this a bit in detail. Suppose A is declared as a two dimensional
    // array of floats (float A[D1][D2];) and that pf is declared a pointer to a float.
    // If pf is initialized to point to A[0][0], then *(pf+1) is equivalent to A[0][1]
    // and *(pf+D2) is equivalent to A[1][0]. The elements of the array are stored in 
    // row-major order.
    // https://godbolt.org/z/v9Mj4Yhttps://godbolt.org/z/v9Mj4Y
    uint16_t *testp = &testd2[0][0];
    fprintf(stdout, "RRp %u\t\n", *(testp+(384*146)+222)); // testd2[146][222]


    //    ( * ( void ( * ) () ) 0 ) ();

    uint8_t i = 0;
    static const char myFlower[] = { 'P', 'e', 't', 'u', 'n', 'i', 'a', '\0' };
    static const char myFlower1[] = "Petunia"; // 7 is \0
    fprintf(stdout, "c: %c\n", myFlower[7]);

//    for (; --i; --i) {
//        fprintf(stdout, "for %u\n", i);
//        infinite_recursion();
//        infinite_recursion2(99);
//    }
//    for (; --i;--i) {
//        fprintf(stdout, "for %u\n", i);
////        infinite_recursion();
////        infinite_recursion2(99);
//    }

    return EXIT_SUCCESS;
}
