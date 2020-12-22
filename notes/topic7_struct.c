#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_MEMBER_LEN 200


int main(void) {

  struct my_struct_type {
    char name[MAX_MEMBER_LEN + 1]; // Note +1 for \0
    size_t age;
  };

  // Stack allocation
  struct my_struct_type db;
  // db.name = "Mickey"; // Invlaid

  char *my_name = "Mickey MS DJB>";
  strcpy(db.name, my_name);
  db.age = 66;


  typedef struct my_struct_type my_struct_type_alias;
  // Stack allocation
  my_struct_type_alias db2;

  typedef struct my_struct_type_new {
    char name[MAX_MEMBER_LEN + 5]; // Note +1 for \0
    size_t age;
    ssize_t round_mark;
  } my_struct_type_new_alias;

  // Stack allocation
  my_struct_type_new_alias db3;
  char *my_new_name = "Mickey MS DJB>AfterChangingPassport";

  // !!! UNDEFINED !!! .name is M, age is 0
  // my_struct_type_new_alias db3_initialized = { *my_new_name, 41, -127 };
  my_struct_type_new_alias db3_initialized = { .name = *my_new_name, .age = 41, .round_mark = -127 }; // Name is M
  strcpy(db3_initialized.name, my_new_name);
  fprintf(stdout, "%p: %s(%p) %zu(%p)\n", &db3_initialized, db3_initialized.name, &db3_initialized.name, db3_initialized.age, &db3_initialized.age);

  db3_initialized.age = 33333;
  fprintf(stdout, "%p: %s(%p) %zu(%p)\n", &db3_initialized, db3_initialized.name, &db3_initialized.name, db3_initialized.age, &db3_initialized.age);

  // Stack allocation
  my_struct_type_new_alias db4 = db3_initialized; // Copy of db3_initialized, not an alias
  db4.age = *(size_t *)&db.age; // take addr of db elem age, deref it as a pointer to size_t and assign resulting value to db4 elem age
  fprintf(stdout, "db4:: %p: %s(%p) %zu(%p)\n", &db4, db4.name, &db4.name, db4.age, &db4.age);

  struct my_struct_type *mallocd_struct;
  mallocd_struct = (struct my_struct_type *)malloc(sizeof(struct my_struct_type) + 1);
  if (mallocd_struct == NULL) perror("mallocd_struct NULL\n");
  (*mallocd_struct).age = 123; // deref address of mallocd_struct, set its elem age to 123
  mallocd_struct->age = 2929; // ->: deref address of mallocd_struct, set its field age to 2929
  strcpy(mallocd_struct->name, "strcpyd_namestrcpyd_namestrcpyd_namestrcpyd_namestrcpyd_namestrcpyd_namestrcpyd_name");
  fprintf(stdout, "mallocd_struct:: %p: %s(%p) %zu(%p)\n", mallocd_struct, mallocd_struct->name, &mallocd_struct->name, mallocd_struct->age, &mallocd_struct->age);

  struct {
    size_t sst1;
    size_t sst2;
  } anon_str1 = { .sst1 = -1 }, anon_str2;

  exit(EXIT_SUCCESS);
}
