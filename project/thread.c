// Thread ctor & dtor
#define Thread_Init (*Thread_Construct)

Thread* Thread_Construct(void)
{
    // Arguments passed to pthread_create
    Thread* t = malloc(sizeof(Thread));
    assert(t);
    _Thread_SetName(t, "myggtk_thread");
    Thread_SetNum(t, 77);
    fprintf(stdout, "class Thread: allocated new object on %p\n", t);
    return t;
}

void Thread_Destruct(Thread* const t)
{
    //    assert(t);
    fprintf(stdout, "class Thread: freed object on %p\n", t);
    free(t);
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
    return t->name;
}

// Private methods
static void _Thread_SetName(Thread* t, char* name)
{
    assert(name);
    strncpy(t->name, name, strlen(name));
}

void* t_print_hello(void* arg)
{
    fprintf(stdout, "class Thread: t_print_hello...\n");
    do_network(arg, 0);
    return "exit_success";
    //    Thread_Destruct(my_thread);
    //    my_thread = NULL;
}

Thread* my_thread = NULL;

ssize_t t_init(void* arg)
{
    my_thread = Thread_Init();
    ssize_t op_status = 0;
    op_status = pthread_attr_init(&my_thread->attr);
    assert(!op_status);

    //    op_status = pthread_attr_setstacksize(&my_thread->attr, 4096);
    //    assert(!op_status);

    op_status = pthread_create(&my_thread->id, &my_thread->attr, &t_print_hello, arg);
    assert(!op_status);

    op_status = pthread_attr_destroy(&my_thread->attr);
    assert(!op_status);

    void* res;
    op_status = pthread_join(Thread_GetId(my_thread), &res);
    assert(!op_status);

    fprintf(stdout, "class Thread: joined id %zu ", Thread_GetNum(my_thread));
    char* ret_res = (char*)res;
    fprintf(stdout, "status %s...\n", ret_res);

    return op_status;
}
