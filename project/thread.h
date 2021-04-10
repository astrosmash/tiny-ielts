// class Thread
#define maxThreadNameLength 255

typedef struct {
    char name[maxThreadNameLength + 1];
    pthread_t id;
    pthread_attr_t attr;
    size_t num;
} Thread;

// Thread ctor & dtor
Thread* Thread_Construct(void);
void Thread_Destruct(Thread* const t);

// Public methods
// Setters
static void Thread_SetId(Thread* const t, pthread_t id);
static void Thread_SetAttr(Thread* const t, pthread_attr_t attr);
static void Thread_SetNum(Thread* const t, size_t num);
// Getters
pthread_t Thread_GetId(Thread* const t);
pthread_attr_t Thread_GetAttr(Thread* const t);
size_t Thread_GetNum(Thread* const t);
static char* Thread_GetName(Thread* const t);

// Private methods
// Setters
static void _Thread_SetName(Thread* t, char* name);

// Definition
#include "thread.c"
