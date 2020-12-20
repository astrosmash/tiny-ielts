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


int main(void)
{
    // stack program-wide (DATA)
    static const uint16_t my_ints[] = { 2, 3, 5, 7, 3, 24198, 138, 65534 /* ;) */, 231, 122, 4 };

    // stack
    const char* my_strings[] = { "ADSKJL2DLUI", "AFKJFJHJKAFKF7", "AFEUIFUEYFAEJFV", "23J2Q3JLKFAFE", "AFJKLFALJF2FQJKFASD", "AVKKGKEFJKFJLKA", "", "AFJDSJFDSJKSDFA", "AFDSKLDFSKLJSDF", "KAFKFD" };

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

    linked_list_t* my_list = (linked_list_t*)malloc(255 * sizeof(linked_list_t));

    for (uint8_t d = 0; d < 255;) {
      my_list->node = (my_array_t*)malloc(sizeof(my_array_t));
      my_list->prev = (linked_list_t*)malloc(sizeof(linked_list_t));
      my_list->next = (linked_list_t*)malloc(sizeof(linked_list_t));

      memcpy( (((my_list->node) + d)->inner), my_array->inner, 20 * sizeof(char)); // or strcpy to copy till \0

      // (*((my_list->node) + d)->inner) = (*(my_array->inner) + d); // &my_array[0], *my_array ??NO MEMORY ALLOCATED at (*(my_list->node)) so cannot dereference and assign a NULL
      ++d;

      my_array_t *pointer_to_cur_array = NULL, *pointer_to_prev_array = NULL;
      pointer_to_cur_array = (my_list->node + d);

      if (pointer_to_cur_array) {
        my_list->prev = (linked_list_t*)malloc(sizeof(linked_list_t));

        my_list->prev->node = pointer_to_cur_array;
        pointer_to_prev_array = my_list->prev->node;

        my_list->next->node = NULL;
        free(my_list->prev);
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
      // free(my_list->prev);
    }

    free(my_list);
    exit(EXIT_SUCCESS);
}
