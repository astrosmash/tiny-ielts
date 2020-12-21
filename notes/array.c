#include <stdio.h>
#include <string.h>

int main(void)
{
    char string[] = "123456780";
    char* str_pointer = string;

    string[6]++;
    // str_pointer++; // str_pointer var now holds an incremented base address by
    // one, e.g. 'cut' first char

    fprintf(stdout, "%p\n", str_pointer);
    fprintf(stdout, "%p\n", (str_pointer + 1));

    fprintf(stdout, "%s\n", str_pointer);

    fprintf(stdout, "%d\n", *str_pointer);
    fprintf(stdout, "%c\n", *(str_pointer + 1));

    *(str_pointer + 1) = 3;
    (*(str_pointer + 2))++;
    fprintf(stdout, "%d\n\n\n", *(str_pointer + 1));

    int ints[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0,
        9, 8, '7', '6', 5, 4, 3, '2', 2, 1 };
    int* int_pointer = ints;
    ints[2]++;

    fprintf(stdout, "%p\n", int_pointer);
    fprintf(stdout, "%p\n", (int_pointer + 1));

    // print address of ints[0]
    fprintf(stdout, "%p\n", int_pointer);

    // dereference the address of ints[0] and represent as array of ints (wchar_t
    // *) - (for some reason this prints blank string)
    // upd - this was just an address, no deref operator (*) was applied
    fprintf(stdout, "Array of ints %d\n", *int_pointer);

    // get address of ints[0], increment by 1 and dereference
    fprintf(stdout, "%i\n", *(int_pointer + 1));
    // first, dereference an address @ int_pointer (ints[0]) and increment by 1
    // (ints[1]); then, dereference resulting address and increment its value (get a copy, +1 to it and write new var to old mem.). If it was arr, no copy would be made and direct val would be incremented (2 insns instead of 3)
    (*(int_pointer + 1))++;

    // dereference address of ints[0]
    fprintf(stdout, "%d\n", *int_pointer);
    // increment by 1 and dereference
    fprintf(stdout, "%d\n", *(int_pointer + 1));

    // take a value of int located at +12 of base address ints
    fprintf(stdout, "%d\n", ints[13]);
    // take address of the variable *(int_pointer + 12) and print as a pointer
    // representatios what is equal to (int_pointer + 12)
    fprintf(stdout, "%p\n", &ints[13]);

    // cannot dereference (*) what is already a sugar of *(int_pointer + 12)
    // fprintf(stdout, "%p\n", *ints[13]);

    return 0;
}
