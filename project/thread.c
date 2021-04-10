// Thread ctor & dtor
#define Thread_Init (*Thread_Construct)

Thread* Thread_Construct(void)
{
    // Arguments passed to pthread_create
    Thread* t = malloc(sizeof(Thread));
    assert(t);
    _Thread_SetName(t, "myggtk_thread");
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
static void Thread_SetId(Thread* const t, pthread_t id)
{
    t->id = id;
}

static void Thread_SetNum(Thread* const t, size_t num)
{
    t->num = num;
}

pthread_t Thread_GetId(Thread* const t)
{
    return t->id;
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

Thread* my_thread = NULL;

ssize_t t_init(void)
{
    my_thread = Thread_Init();
    return EXIT_SUCCESS;
}

void t_print_hello(void)
{
    fprintf(stdout, "class Thread: t_print_hello...\n");

    Thread_Destruct(my_thread);
    my_thread = NULL;
}
