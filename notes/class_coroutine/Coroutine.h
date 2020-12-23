#include <assert.h>
#include <stdbool.h>
#include <string.h>

// Coroutine general class
typedef struct {
    bool done;
    bool finished;
    void *arg;
    jmp_buf invoking_env;
    jmp_buf cr_env;
    jmp_buf error_env; // Used for stacktrace on errors
    error_handler_callback_t *errcb; // To handle the above
} coroutine_t;

// Coroutine function
typedef void (*coroutine_callback_t)(coroutine_t * const cr);

// Error handler for a stacktrace
typedef void (*error_handler_callback_t)(jmp_buf *err_buf, const char *fmt, ...);

// Coroutine class public methods
coroutine_t *Coroutine_Ctor(coroutine_callback_t *cb, error_handler_callback_t *ecb);
#define Coroutine_Init *Coroutine_Ctor
void Coroutine_Dtor(coroutine_t * const cr);

void Cr_SwitchToCoroutine(coroutine_t * const cr);
void Cr_YieldFromCoroutine(coroutine_t * const cr);
void Cr_IsPrimeCoroutine(coroutine_t * const cr);

// Coroutine class private methods
static void _Cr_IsPrimeInit(coroutine_t * const cr);
static void _Cr_IsPrimeNext(coroutine_t * const cr);
static void _Cr_IsPrimeEmpty(coroutine_t * const cr);
static void _Cr_HandleError(coroutine_t * const cr);

// Staticmethods
void SwapJmpBufs(jmp_buf *from, jmp_buf *to);

// Copy-paste macro
// CR_START does the initial jmp_buf setup only if this is the base coroutine (we check using the init_done member)
#define CR_START(cr) bool __is_base_cr = !cr->init_done; do { if ( !cr->init_done && (0 == setjmp(cr->cr_env)) ) { return; } } while (0)
#define CR_YIELD(cr) Cr_YieldFromCoroutine(cr)
#define CR_END(cr) while(__is_base_cr) { cr->finished = true; CR_YIELD(cr); }
