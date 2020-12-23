error_handler_callback_t *error(jmp_buf *err_buf, const char *fmt, ...) {
  assert(buf);
  assert(fmt);

  va_list ap;
  va_start(ap, fmt);

  vfprintf(stderr, fmt, ap);
  putc('\n', stderr);

  va_end(ap, fmt);
  longjmp(*err_buf, 1);
}

void SwapJmpBufs(jmp_buf *from, jmp_buf *to) {
  assert(from);
  assert(to);

  if (setjmp(*from) == 0) {
    longjmp(*to, 1);
  }
}

coroutine_t *Coroutine_Ctor(coroutine_callback_t *cb, error_handler_callback_t *ecb) {
  assert(cb);
  assert(ecb);

  register void *SP asm ("sp");
  void *local_SP = NULL;
  void *remote_SP = NULL;
  size_t stack_used = 0;
  uint32_t i = 0;

  coroutine_t *new_coroutine = (coroutine_t *)malloc(sizeof(coroutine_t) + 1);
  new_coroutine->done = false;
  new_coroutine->finished = false;
  new_coroutine->arg = NULL;
  new_coroutine->errcb = ecb;
  new_coroutine->invoking_env = NULL;
  new_coroutine->cr_env = NULL;
  new_coroutine->error_env = NULL;

  static const uint8_t jmp_buf_spi = 8;
  local_sp =

}

void Cr_SwitchToCoroutine(coroutine_t * const cr) {
  assert(cr);
  SwapJmpBufs(&cr->invoking_env, &cr->cr_env);
}

void Cr_YieldFromCoroutine(coroutine_t * const cr) {
  assert(cr);
  SwapJmpBufs(&cr->cr_env, &cr->invoking_env);
}
