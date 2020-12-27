#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    char inner[99];
} my_array_t;

typedef struct linked_list {
    my_array_t* node;
    // prev is optional. Here to use pointer as base type decorator - http://c-faq.com/decl/charstarws.html
    // note that linked_list_t *prev, *next; won't work - typedef declaration can not be used until it is defined (http://c-faq.com/decl/selfrefstruct.html)
    struct linked_list *prev, *next;
} linked_list_t;

void set_int(int *ptr_to_int)
{
  // deref ptr_to_int, set val to 12
  // cannot be checked that it's an actual ptr to int!
  // (https://wiki.sei.cmu.edu/confluence/display/c/EXP34-C.+Do+not+dereference+null+pointers?focusedCommentId=88020518#comment-88020518)
  *ptr_to_int = 12;
}

// we call set_int from within allocate_int, so stack access is ok
void allocate_int(void)
{
  int my_int /* = 0 */, *ptr_to_my_int = &my_int; // pointer to 0
  set_int( ptr_to_my_int + 1 ); // >>3 bytes + 1 byte
}

int main(void)
{
    void (*myfuncptr)(); // declare myfuncptr as a pointer to func returning void and taking no args
    size_t (**myfuncptr2)(char*, size_t); // declare myfuncptr2 as a pointer to pointer to func [pointer to a first elem of an array of pointers to func] returning size_t and taking char*, size_t as args

    // stack program-wide (DATA)
    static const uint16_t my_ints[] = { 2, 3, 5, 7, 3, 24198, 138, 65534 /* ;) */, 231, 122, 4 };

    // stack
    const char *my_strings[] = { "ADSKJL2DLUI", "AFKJFJHJKAFKF7", "AFEUIFUEYFAEJFV", "23J2Q3JLKFAFE", "AFJKLFALJF2FQJKFASD", "AVKKGKEFJKFJLKA", "", "AFJDSJFDSJKSDFA", "AFDSKLDFSKLJSDF", "KAFKFD" };
    fprintf(stdout, "--\n\n%s\n\n", my_strings[3]);;

    // stack
    my_array_t my_array[] = {
      { .inner = "ADSKJL2DLUI" },
      { .inner = "AFKJFJHJKAFKF7" },
      { .inner = "AFEUIFUEYFAEJFV" },
      { .inner = "23J2Q3JLKFAFE" },
      { .inner = "AFJKLFALJF2FQJKFASD" },
      { .inner = "AFEUIFUEYFAEJFV" },
      { .inner = "AVKKGKEFJKFJLKA" },
      { .inner = "AFJDSJFDSJKSDFA" },
      { .inner = "AFJKLFALJF2FQJKFASD" },
      { .inner = "AVKKGKEFJKFJLKA" },
      { .inner = "23J2Q3JLKFAFE" },
      { .inner = "AFEUIFUEYFAEJFV" },
      { .inner = "AFKJFJHJKAFKF7" },
      { .inner = "AVKKGKEFJKFJLKA" },
    };

    linked_list_t *my_list = (linked_list_t*)malloc(255 * sizeof(linked_list_t));

    for (uint8_t d = 0; d < (sizeof(my_array) / sizeof(*my_array)) ;) {

      my_list->node = (my_array_t*)malloc(sizeof(my_array_t));
      my_list->prev = (linked_list_t*)malloc(sizeof(linked_list_t));
      my_list->prev->node = (my_array_t*)malloc(sizeof(linked_list_t));
      my_list->next = (linked_list_t*)malloc(sizeof(linked_list_t));
      my_list->next->node = (my_array_t*)malloc(sizeof(linked_list_t));

      memcpy( ((my_list->node) + d)->inner, &my_array[d].inner, strlen(my_array[d].inner)); // or strcpy to copy till \0
      fprintf(stdout, "%s",  (((my_list->node) + d)->inner));
      fprintf(stdout, "\n");

      memcpy( ((my_list->next->node) + d)->inner, &my_array[d + 1].inner, strlen(my_array[d + 1].inner)); // or strcpy to copy till \0
      memcpy( ((my_list->prev->node) + d)->inner, &my_array[d - 1].inner, strlen(my_array[d - 1].inner)); // or strcpy to copy till \0

      char *myptr = (my_list->node + d)->inner;
      char **ptrtoptr = &myptr; // ptr to ptr to char, or void* ptr to char (== void* ptrtoptr = &myptr)
      void *ptrtoptrtoptr = &ptrtoptr; // (== char ***ptrtoptrtoptr = &ptrtoptr)
      // char (*daytab2)[13] = (char [13]){
      //   {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
      // };
      char (*daytab)[13] = (char [][13]){ // arr of 13 ptrs to char (this would be ptr to 72 w/o a cast)
          {72, 72, 72, 72, 72, 72, 72, 31, 30, 31, 30, 31},
          {74, 74, 74, 74, 74, 74, 72, 31, 30, 31, 30, 31}
      };

      /*[3]*/char (*suits)[4][3] = (char [][4][3]){ {"abc", "cab", "bca2"}, {"abc2", "cab2", "bc2"} }; // array from 4 ptrs to 3 chars
      const char *suits_sugar[2][3] = { {"abc", "cab", "bca"}, {"abc2", "cab2", "bca2"} }; // suits_sugar an array of 2 ptrs to 3 const chars (3 char*)

      fprintf(stdout, "-----------------------------------\n");
      fprintf(stdout, "%c\n",  myptr[4]);
      fprintf(stdout, "%c\n",  *(myptr + 4));
      fprintf(stdout, "%c\n",  myptr[7]);
      fprintf(stdout, "%p\n",  (myptr + 7));
      fprintf(stdout, "%p\n",  ptrtoptr);
      fprintf(stdout, "%p\n",  ptrtoptrtoptr);
      fprintf(stdout, "%p\n",  daytab);

      fprintf(stdout, "!word: %s\n",  *daytab); // deref an address of array of 13 chars (get address of first elem of array of 13 chars and get its val (print as a string to \0))
      fprintf(stdout, "!word2: %s\n",  *(daytab + 1));

      fprintf(stdout, "!char %c\n",  **daytab); // deref an address of first member of first array of 13 chars (get address of first elem of array of 13 chars, get address of its first elem [pointer to pointer to char] and dereference as a char))
      fprintf(stdout, "!char2 %c\n",  **(daytab + 1)); // deref an address of first member (Char) of second array of 13 chars
      fprintf(stdout, "!char7 %c\n",  *(*(daytab + 1) + 6)); // deref an address of seventh member (Char) of second array of 13 chars

      fprintf(stdout, "!word: %s\n",  daytab[1]); // %s wants a pointer to a str - use sugar (deref address of first elem of pointers to char) - JJJJJJH
      fprintf(stdout, "!char %c\n",  daytab[1][2]); // deref address of first elem of pointers to char, deref address of third elem (pointer to char) - 74/J


      fprintf(stdout, "!suits: %s\n",  *suits_sugar[1]);
      fprintf(stdout, "!suits2: %s\n",  *suits_sugar[0]);
      fprintf(stdout, "!char3: %c\n",  *(*suits_sugar[1] + 2));
      fprintf(stdout, "-----------------------------------\n");

      // (*((my_list->node) + d)->inner) = (*(my_array->inner) + d); // &my_array[0], *my_array ??NO MEMORY ALLOCATED at (*(my_list->node)) so cannot dereference and assign a NULL
      ++d;


      my_array_t *pointer_to_cur_array = NULL, *pointer_to_prev_array = NULL;

      pointer_to_cur_array = (my_list->node + d);
      //
      if (pointer_to_cur_array) {
      //   // my_list->prev = (linked_list_t*)malloc(sizeof(linked_list_t));
      //
      //   my_list->prev->node = pointer_to_cur_array;
      //   memcpy(my_list->prev->node->inner, pointer_to_cur_array->inner, strlen(pointer_to_cur_array->inner));
      //
      //   // my_list->prev->node->inner = pointer_to_cur_array->inner;
        pointer_to_prev_array = my_list->prev->node;
      //
      //   memcpy(pointer_to_prev_array->inner, my_list->prev->node->inner, strlen(my_list->prev->node->inner));
      //   // pointer_to_prev_array->inner = my_list->prev->node->inner;
      //
      //   // my_list->next->node = NULL;
      //   // free(my_list->prev);
      //   // my_list->prev = NULL; // dangling pointer null
      }

      if (!pointer_to_cur_array || !pointer_to_prev_array) {
        fprintf(stderr, "aliased pointers NULL");
        exit(EXIT_FAILURE); // malloc's not freed
      }

      fprintf(stdout, "%u", d);
      fprintf(stdout, "\n");

      fprintf(stdout, "cur: %s(%p)", pointer_to_cur_array->inner, pointer_to_cur_array);
      fprintf(stdout, "\n");
      fprintf(stdout, "\n");

      fprintf(stdout, "prev: %s(%p)", pointer_to_prev_array->inner, pointer_to_prev_array);
      fprintf(stdout, "\n");
      fprintf(stdout, "\n");

      free(my_list->node);
      free(my_list->next);
      free(my_list->next->node);
      free(my_list->prev);
    }

    free(my_list);


    const char *my_str_1 = "MY_STR_ZZZZ1";
    const char *my_str_2 = "MY_STR_2";
    char *my_str_3 = NULL; // if init as "" (str), we'll get strcpy bus error as we cannot modify string literals
    my_str_3 = malloc(strlen(my_str_2) + 1); // not freeing and not removing dangling ptr

    char *my_str_4 = NULL;

    // This is pointer to the strncat destination array, which should contain a C string
    // and should be large enough to contain the concatenated resulting string which includes the additional null-character.
    my_str_4 = malloc(3 * strlen(my_str_1) + 1); // not freeing and not removing dangling ptr


    uint32_t my_str_1_len = strlen(my_str_1);
    uint32_t sizeof_ptr = sizeof(my_str_1); // 8 byte sizeof(pointer to const char) - pointers are WORD-size(64bit)
    uint32_t sizeof_str = sizeof(*my_str_1); // 1 byte sizeof(const char)

    strcpy(my_str_3, my_str_2);
    strncpy(my_str_3, my_str_2, strlen(my_str_3));

    strcat(my_str_4, my_str_2);
    strncat(my_str_4, my_str_1, strlen(my_str_1));

    int8_t strcmp_res = strcmp(my_str_1, my_str_2);
    if (strcmp_res) {
      if (strcmp_res < 0) {
        fprintf(stdout, "%s is less than %s \n", my_str_1, my_str_2);
      } else if (strcmp_res > 0) {
        fprintf(stdout, "%s is more than %s \n", my_str_1, my_str_2);
      }
    } else {
      fprintf(stdout, "%s is equal to %s \n", my_str_1, my_str_2);
    }

    fprintf(stdout, "my_str_1_len: %u \n", my_str_1_len);
    fprintf(stdout, "my_str_1_sizeof: %u \n", sizeof_str);
    fprintf(stdout, "my_str_1_ptr_sizeof: %u \n", sizeof_ptr);
    fprintf(stdout, "my_str_3: %s \n", my_str_3);
    fprintf(stdout, "my_str_4: %s \n", &my_str_4[0]);

    char *intstr[] = { "234", "567", "890", "123", "4545", "5656", "7878", "8989", "9090", "110110", "111220"}; // 11 pointers to char*
    fprintf(stdout, "intstr_arr %p \n", intstr);
    fprintf(stdout, "intstr %s \n", *intstr);
    fprintf(stdout, "intstr[2] %s \n", *(intstr + 2));

    fprintf(stdout, "sizeof(intstr) %lu\n", sizeof(intstr));
    fprintf(stdout, "sizeof(*intstr) %lu\n", sizeof(*intstr));

    for (uint8_t i = 0; i < (sizeof(intstr) / sizeof(*intstr)); ++i) { // 11*8 = 88bytes total / 8 per pointer = 11 pointers
      uint64_t digit = 0;
      digit = atoi(*(intstr + i));
      fprintf(stdout, "digit%u: %llu \n", i, digit);
      fprintf(stdout, "digit%u: %u\n", i, atoi("2323232"));
    }
    char *tailptr = NULL;
    uint32_t digit_strtol = strtol(*(intstr + 3), &tailptr, 10);
    fprintf(stdout, "digit_strtol: %u \n", digit_strtol);




    exit(EXIT_SUCCESS);
}
