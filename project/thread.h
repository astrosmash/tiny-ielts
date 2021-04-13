// class Thread
#define maxThreadNameLength 255

typedef struct {
    char name[maxThreadNameLength + 1];
    pthread_t id;
    pthread_attr_t attr;
    size_t num;
} Thread;

// Thread ctor & dtor
Thread* Thread_Construct(void* (*)(void*), void*);
void Thread_Destruct(Thread**);

// Public methods
// Setters
static void Thread_SetId(Thread* const, pthread_t);
static void Thread_SetAttr(Thread* const, pthread_attr_t);
static void Thread_SetNum(Thread* const, size_t);
// Getters
pthread_t Thread_GetId(Thread* const);
pthread_attr_t Thread_GetAttr(Thread* const);
size_t Thread_GetNum(Thread* const);
static char* Thread_GetName(Thread* const);
ssize_t Thread_Join(Thread* const, void*);

// Private methods
// Setters
static void _Thread_SetName(Thread*, char*);

// Definition
#include "thread.c"
