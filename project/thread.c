// Thread ctor & dtor
#define Thread_Init (*Thread_Construct)

Thread* Thread_Construct(void* (*func_addr)(void*), void* func_arg)
{
    // Arguments passed to pthread_create
    Thread* t = malloc(sizeof(Thread));
    assert(t);
    assert(func_addr);
    assert(func_arg);
    memset(t, 0, sizeof(*t));

    _Thread_SetName(t, "myggtk_thread");
    Thread_SetNum(t, 77);
    debug("allocated new object on %p\n", (void*)t);

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
    debug("freed object on %p\n", (void*)*t);
    free(*t);
    *t = NULL;
}

// Public methods
// Setters

static void Thread_SetId(Thread* const t, pthread_t id)
{
    t->id = id;
}

static void Thread_SetAttr(Thread* const t, pthread_attr_t attr)
{
    t->attr = attr;
}

static void Thread_SetNum(Thread* const t, size_t num)
{
    t->num = num;
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

size_t Thread_GetNum(Thread* const t)
{
    return t->num;
}

char* Thread_GetName(Thread* const t)
{
    assert(t);
    return t->name;
}

ssize_t Thread_Join(Thread* const t, void* res)
{
    debug("joining %s\n", Thread_GetName(t));
    return pthread_join(Thread_GetId(t), res);
}

// Private methods

static void _Thread_SetName(Thread* t, char* name)
{
    assert(name);
    strncpy(t->name, name, strlen(name));
}
