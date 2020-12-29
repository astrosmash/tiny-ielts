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
    fprintf(stdout, "BB %u\t\n", *(*(testd2 + 1) + 1)); // testd2[1][1]
    fprintf(stdout, "ZZ %u\t\n", *(*(testd2 + 1) + 2)); // testd2[1][2]

    //    ( * ( void ( * ) () ) 0 ) ();

    uint8_t i = 0;

//    for (; --i; --i) {
//        fprintf(stdout, "for %u\n", i);
//        infinite_recursion();
////        infinite_recursion2(99);
//    }
    for (; --i;--i) {
        fprintf(stdout, "for %u\n", i);
//        infinite_recursion();
//        infinite_recursion2(99);
    }

    return EXIT_SUCCESS;
}
