// class Thread
#define maxThreadNameLength 255

typedef struct {
    char name[maxThreadNameLength + 1];
    pthread_t id;
    pthread_attr_t attr;
} Thread;

// Thread ctor & dtor
Thread* Thread_Construct(void* (*)(void*), void*);
void Thread_Destruct(Thread**);

// Getters
pthread_t Thread_GetId(Thread* const);
pthread_attr_t Thread_GetAttr(Thread* const);
static char* Thread_GetName(Thread* const);

// Public methods
ssize_t Thread_Join(Thread* const, void*);

// Private methods
static void _Thread_SetName(Thread*, char*);

// Definition
#include "thread.c"
