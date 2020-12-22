#include <stdio.h>

void contains_null_check(int *P) {
  int dead = *P;
    fprintf(stdout, "%d", *P);

  if (P == 0)
    return;
  *P = 4;
}

int main(void) {
    int my_int = 9;
    int *int_ptr = &my_int;
    contains_null_check(int_ptr);
}
