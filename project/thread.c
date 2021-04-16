// Thread ctor & dtor
#define Thread_Init (*Thread_Construct)

// Function location and args to it to run in the thread
Thread* Thread_Construct(void* (*func_addr)(void*), void* func_arg)
{
    // Arguments passed to pthread_create
    Thread* t = malloc(sizeof(Thread));

    assert(t);
    assert(func_addr);
    assert(func_arg);

    memset(t, 0, sizeof(*t));

    _Thread_SetName(t, "2ch_worker");
    debug(3, "Allocated new object on %p\n", (void*)t);

    ssize_t op_status = 0;
    op_status = pthread_attr_init(&t->attr);
    assert(!op_status);

    op_status = pthread_create(&t->id, &t->attr, func_addr, func_arg);
    assert(!op_status);

    op_status = pthread_attr_destroy(&t->attr);
    assert(!op_status);

    return t;
}

void Thread_Destruct(Thread** t)
{
    assert(*t);
    debug(3, "Freed object on %p\n", (void*)*t);
    free(*t);
    *t = NULL;
}

// Getters
pthread_t Thread_GetId(Thread* const t)
{
    return t->id;
}

pthread_attr_t Thread_GetAttr(Thread* const t)
{
    return t->attr;
}

char* Thread_GetName(Thread* const t)
{
    return t->name;
}

// Public methods
ssize_t Thread_Join(Thread* const t, void* res)
{
    debug(3, "Joining %s\n", Thread_GetName(t));
    return pthread_join(Thread_GetId(t), res);
}

// Private methods
static void _Thread_SetName(Thread* t, char* name)
{
    assert(name);
    strncpy(t->name, name, strlen(name));
}

