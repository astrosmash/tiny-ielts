In the strictest sense of the word, everything in C is pass-by-value. This often confuses beginning C programmers,
especially when it comes to pointers, arrays, and structs. So what do we mean when we say pass-by-value and pass-by-reference.

When we pass-by-value we are passing a copy of the variable to a function. When we pass-by-reference we are passing an alias of the
variable to a function. C can pass a pointer into a function but that is still pass-by-value.
It is copying the value of the pointer, the address, into the function.
https://denniskubes.com/2012/08/20/is-c-pass-by-value-or-reference/

------------------------------------------------------------------------------------------------------------------------------------
"C++" is a string literal stored in read only location and hence cannot be modified. With this - char* t="C++";
// t is pointing to a string literal stored in read only location
Instead, you should have -
char t[] = "C++" ;  // Copying the string literal to array t

------------------------------------------------------------------------------------------------------------------------------------
*(*(a + i) + j)
which is same as,

a[i][j]

------------------------------------------------------------------------------------------------------------------------------------
Hence in the assignment to a, the 8th character of the array is taken by offsetting the value of array_place by 7,
and moving the contents pointed to by the resulting address

Here variable arr will give the base address, which is a constant pointer pointing to the first element of the array, arr[0].
Hence arr contains the address of arr[0] i.e 1000. In short, arr has two purpose - it is the name of the array and it acts as
a pointer pointing towards the first element in the array.

When an array name is passed to a function, what is passed is the location of the initial element. Within the called function, 
this argument is a local variable, and so an array name parameter is a pointer, that is, a variable containing an address.
https://eli.thegreenplace.net/2009/10/21/are-pointers-and-arrays-equivalent-in-c/

------------------------------------------------------------------------------------------------------------------------------------
We can also declare a pointer of type int to point to the array arr.

int *p;
p = arr;  
// or, 
p = &arr[0]; //both the statements are equivalent.

Now we can access every element of the array arr using p++ to move from one element to another.
NOTE: You cannot decrement a pointer once incremented. p-- won't work.

------------------------------------------------------------------------------------------------------------------------------------
In C99 you can do this, though (if not static at least):

char (*daytab)[13] = (char [][13]){
    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, 
    {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};
btw не знал что каст простой решает проблему отброса последующих кроме первого значения при скалярной инициализации массива
как указатель на первый элемент

------------------------------------------------------------------------------------------------------------------------------------
Asterisk on Lvalue (var definition) : create a var to store an address of type X 
GENERAL MEANING Asterisk (on rvalue): go and dereference the value located at this address.
Ampersand on rvalue: take the address of this variable (read where it's located).

------------------------------------------------------------------------------------------------------------------------------------
Declare pointer: say what type of value in that address.
https://courses.caslab.queensu.ca/cisc220a/wp-content/uploads/sites/24/2016/11/ptrs.pdf

------------------------------------------------------------------------------------------------------------------------------------
const char *suits[]={"abc","cab","bca"}
cout<<*suits[1];
// c

(abc\0, cab\0 - %s string looks for \0)

------------------------------------------------------------------------------------------------------------------------------------
type char can hold values up to 127/255;
types short int and int can hold values up to 32,767/65k; and
type long int can hold values up to 2,147,483,647/4M.
 sizeof(char) <= sizeof(short) <= sizeof(int) <= sizeof(long) <= sizeof(long long)
fixed size types: https://en.wikibooks.org/wiki/C_Programming/stdint.h#Exact-width_integer_types

------------------------------------------------------------------------------------------------------------------------------------
http://c-faq.com/sx1/index.html#declarator
The ``second half'' of a C declaration, consisting of an identifier name along with optional *, [], or () syntax indicating
(if present) that the identifier is a pointer, array, function, or some combination.

------------------------------------------------------------------------------------------------------------------------------------
what does typedef int (*funcptr)(); mean?
A: It defines a typedef, funcptr, for a pointer to a function (taking unspecified arguments) returning an int.
It can be used to declare one or more pointers to functions: funcptr pf1, pf2;
which is equivalent to the more verbose, and perhaps harder to understand int (*pf1)(), (*pf2)();
http://c-faq.com/decl/pfitypedef.html

------------------------------------------------------------------------------------------------------------------------------------
typedef char *charp; - declare charp as a pointer to char and create an alias to pointer to char as charp
charp s = "s";
const charp s = const pointer to char s

instead: typedef const char *charp - pointer to const char

------------------------------------------------------------------------------------------------------------------------------------
Q: What's the difference between const char *p, char const *p, and char * const p?
Type qualifiers like const can affect a pointer variable in two (or more) different ways: either the pointer can be qualified,
or the value pointed to. 

A: The first two are interchangeable; they declare a pointer to a constant character (you can't change any pointed-to characters).
char * const p declares a constant pointer to a (variable) character (i.e. you can't change the pointer).

------------------------------------------------------------------------------------------------------------------------------------
static functions are functions that are only visible to other functions in the same file (more precisely the same translation unit).

------------------------------------------------------------------------------------------------------------------------------------
array of N pointers to functions returning pointers to functions (*(*a[N])())()

------------------------------------------------------------------------------------------------------------------------------------
array of N pointers to functions returning pointers to functions returning pointers to char  char *(*(*a[N])())();

------------------------------------------------------------------------------------------------------------------------------------
char *pc; the base type is char, the identifier is pc, and the declarator is *pc; this tells us that *pc is a char ( "get the contents of the address"(dereference pc) and get a char)

------------------------------------------------------------------------------------------------------------------------------------
http://c-faq.com/decl/cdecl1.html

------------------------------------------------------------------------------------------------------------------------------------
An array is a constant pointer.
int array[100]; // constant pointer
int *ptr; // uninitialized pointer
...
ptr = array; /* this is legal */
array = ptr; /* this is not legal */

------------------------------------------------------------------------------------------------------------------------------------
Since an array is a pointer, you can use array notation with a pointer.
int *intP;
intP = .....
/* The following is legal: */
int val = intP[5] + intP[17];
intP[2] = 93;

In C, you can do arithmetic with pointers:
*(intP+5)
Adds 5*sizeof(int) to pointer, then returns contents of that location

------------------------------------------------------------------------------------------------------------------------------------
Sometimes you want a pointer to a memory location butdon't know what type of data is in it
Use type void*.

Rules about void*:
• can't dereference a void* value
• can assign or cast a void* value to any other pointer type

The declaration void *somePointer; is used to declare a pointer of some nonspecified type. You can assign a value to a void pointer,
but you must cast the variable to point to some specified type before you can dereference it. Pointer arithmetic is also not valid
with void * pointers.
------------------------------------------------------------------------------------------------------------------------------------
void *genericPtr;
int x = 7;
genericPtr = &x;
printf("%d\n", *genericPtr); Illegal!

int *intptr = genericPtr; /* no cast needed */
printf("%d\n", *intptr); Legal

float f = 3.14159;
genericPtr = &f;
printf("%f\n", *((float *)genericPtr)); Legal

------------------------------------------------------------------------------------------------------------------------------------
#define NULL (void *) 0

------------------------------------------------------------------------------------------------------------------------------------
In simple cases, a 32 bit processor will have a 32 bit "word" size (and pointer size).
A 64 bit processor will have a 64 bit "word" size (and pointer size).
so size of a pointer is 8 bytes on x64.

------------------------------------------------------------------------------------------------------------------------------------
Memory Management
Three kinds of memory allocation in C:

1. static allocation: global variables – space is allocated at start of program, never freed
2. automatic allocation (the call stack): parameters and local variables inside functions – space is allocated when function
is called, freed when function returns
3. dynamic allocation (heap): program asks for space using call to C library function (malloc). Space is reserved until program
explicitly frees it (using free).

ALSO ALWAYS NULL DANGLING POINTERS!!

------------------------------------------------------------------------------------------------------------------------------------
How much elements are in the array? sizeof(arr) / sizeof(arr[0]).

------------------------------------------------------------------------------------------------------------------------------------
Differences between malloc and calloc:
1. calloc takes two parameters(1. number of elements, 2. size of individual element), malloc takes one (total size)
2. calloc zeros contents of heap array (memset 0 before allocation); malloc does not

------------------------------------------------------------------------------------------------------------------------------------
Example: it takes 5 characters to store "CISC":
0. 'C'
1. 'I'
2. 'S'
3. 'C'
4. '\0'

------------------------------------------------------------------------------------------------------------------------------------
Using scanf
char name[21];
printf("what is your name? ");
scanf("%20s", name);

What does scanf do?
• skips white space
• reads non-white characters & copies into name
• stops after 20 characters or when it reaches a "white space" character

Important: field width for %s = number of characters to read.
Does not count the ending null!
Dangerous alternative:
scanf("%s", name);

------------------------------------------------------------------------------------------------------------------------------------
Appendix B3 in Kernighan & Ritchie: functions in <string.h>

strlen(s): length of s
strcpy(s1,s2): copies s2 to s1
strncpy(s1,s2,n)
strcat(s1,s2): s1 = s1 concatenated with s2
strncat(s1,s2,n)
strcmp(s1,s2): compares s1 & s2 alphabetically
https://en.wikibooks.org/wiki/C_Programming/Procedures_and_functions

------------------------------------------------------------------------------------------------------------------------------------
you can't modify string literals. It's undefined behavior.
https://wiki.sei.cmu.edu/confluence/display/c/STR30-C.+Do+not+attempt+to+modify+string+literals
(As an array initializer, it's ok. As a pointer to char, its not)

------------------------------------------------------------------------------------------------------------------------------------
char name[5]; << note 5 to store \0 after Fred
strcpy(name, "Fred"); // Fred\0  is 5 bytes

------------------------------------------------------------------------------------------------------------------------------------
strncpy(str1, str2, n):
Copies at most n characters from str2 to str1.
Does not automatically add \0!!!

Safe practice:
strncpy(str1, str2, n);
!!!!!! str1[n] = '\0'; !!!!!!
Safer Copying Function: strncpy

Advice:
• Use strcpy when you're sure it's safe.
• Use strncpy when you're not sure.
• With strncpy, careful about the ending null

Constant refrain:
Make sure you have enough room
Make sure every string has a null on the end

------------------------------------------------------------------------------------------------------------------------------------
You can declare main with two parameters:
argc: the number of command-line arguments
argv: array of the command-line arguments

Notes:
. There is always at least one argument. The first one (argv[0]) is the name of the program
• argv is an array of strings

------------------------------------------------------------------------------------------------------------------------------------
int x = (int) strtol(str, &tail, 10);
Strings To Numbers

long int strtol(char *string,
 char **tailptr,
 int base)

string: the string to parse into an int
tailptr: used to return a pointer to the remainder of the string (whatever came after the integer)
base: the radix, usually 10
strtol ignores white space at the beginning of string

Detecting errors:
• If characters after whitespace are not a legal integer, x will be zero and tail will be equal to str – no characters used.
• If there were no extra characters in str after the integer, *tail will be '\0'.

Similar function for floating-point numbers:
double strtod(const char *string,
 char **tailptr)

------------------------------------------------------------------------------------------------------------------------------------
To declare f as a pointer to a function that takes a integer parameter and returns a float:
float (*f) (int);
The parenthesis are necessary!


Pointer to a function that takes two floats and returns a string:
(char *) (*g) (float, float);

------------------------------------------------------------------------------------------------------------------------------------
SIZE_MAX is the largest possible value that a size_t could take, so it is not possible to have anything larger than SIZE_MAX.

The test was added to catch the possibly theoretical situation where the length of input_str was somehow the maximum size
for size_t, and adding one to this size in the malloc expression (to allocated space for the trailing null byte) results in
an integer overflow. https://wiki.sei.cmu.edu/confluence/display/c/EXP34-C.+Do+not+dereference+null+pointers

------------------------------------------------------------------------------------------------------------------------------------
For unions, C reserves space for a largest type of union.

union {
 int i;
 char s[10];
 double d;
} mystery;

Meaning: mystery is an area in memory that may be used for one of the following:
• an integer or
• 10 characters or
• a double

------------------------------------------------------------------------------------------------------------------------------------
length of str is calculated using strlen only. sizeof is wrong

char *s = "ss";

sizeof(s)  - size of a ptr
sizeof(*s) - deref 1st char - sizeof a chaf

------------------------------------------------------------------------------------------------------------------------------------
char *(*(*a[])())() a is an >>> array <<< of pointers to function that returns pointer to function that returns ptr to char

------------------------------------------------------------------------------------------------------------------------------------
The star is used to specify the width of the value and printf takes then 3 arguments : printf("%*d", width, value);.
Example : printf("%*d", 3, 16)

------------------------------------------------------------------------------------------------------------------------------------
char str[] = "abc"; // declares char[4] initialized to 'a','b','c','\0'

// array of 2 arrays of 3 ints each
int a[2][3] = {{1,2,3},  // can be viewed as a 2x3 matrix
               {4,5,6}}; // with row-major layout

------------------------------------------------------------------------------------------------------------------------------------
Sorting  https://levelup.gitconnected.com/sorting-algorithms-selection-sort-bubble-sort-merge-sort-and-quicksort-75479f8f80b1

fast:
merge sort (Θ(n log n) - divide dataset into smaller dses and sort them and merge them
(sort left first, then right first, then merge, ++) It compares first elements of DSes before merge to find smaller one.)
quick sort (qsort) - : Ω(n log(n)) — O(n²) - break ds into 2 parts, get a pivot number, iterate over chinks & place smaller numbers
to the left and larger ones to the right of the pivot

slow:
bubble (O(n²) — Ω(n) ), selection sort (Θ(n²))

------------------------------------------------------------------------------------------------------------------------------------
Writing Lines
int fputs (char *s, FILE *stream)
Writes s to the file.
Does not write null or end of line at the end.
If error: return value is EOF.

int puts (char *s)
Writes s to the standard output file – with '\n' at the end.
If error: return value is EOF

------------------------------------------------------------------------------------------------------------------------------------
stdX are FILE *

------------------------------------------------------------------------------------------------------------------------------------
Result of fclose is an int:
• 0 if file was closed successfully
• EOF if an error occured

------------------------------------------------------------------------------------------------------------------------------------
do not use sprintf, use snprintf instead.

------------------------------------------------------------------------------------------------------------------------------------
If you declare int i; as a (non-static) local variable inside of a function, it has an indeterminate value.
It is uninitialized and you can't use it until you write a valid value to it.

------------------------------------------------------------------------------------------------------------------------------------
int (**(*)(void))() ptr to function that accepts void and returns ptr to ptr to func that accepts no args and returns int

------------------------------------------------------------------------------------------------------------------------------------
An extern array of unspecified size is an incomplete type; you cannot apply sizeof to it.
sizeof operates at compile time, and there is no way for it to learn the size of an array which is defined in another file.
also sizeof does not show size of malloc'ed block.

------------------------------------------------------------------------------------------------------------------------------------
char *a[4] = {"this", "is", "a", "test"};
char **p = a;

DYNAMIC allocation:
#include <stdlib.h>
char **p = malloc(4 * sizeof(char *));
if(p != NULL) {
 p[0] = malloc(5);
 p[1] = malloc(3);
 p[2] = malloc(2);
 p[3] = malloc(5);

 if(p[0] && p[1] && p[2] && p[3]) {
  strcpy(p[0], "this");
  strcpy(p[1], "is");
  strcpy(p[2], "a");
  strcpy(p[3], "test");
 }
}

------------------------------------------------------------------------------------------------------------------------------------
Q: What am I allowed to assume about the initial values of variables and arrays which are not explicitly initialized?
If global variables start out as ``zero'', is that good enough for null pointers and floating-point zeroes?

Uninitialized variables with static duration
(that is, those declared outside of functions, and those declared with the storage class static), are guaranteed to start out
as zero, just as if the programmer had typed ``= 0'' or ``= {0}''. Therefore, such variables are implicitly initialized to the
null pointer (of the correct type; see also section 5) if they are pointers, and to 0.0 if they are floating-point. 

!!Variables with automatic duration (i.e. local variables without the static storage class) start out containing garbage, unless
they are explicitly initialized. (Nothing useful can be predicted about the garbage.) If they do have initializers, they are
initialized each time the function is called (or, for variables local to inner blocks, each time the block is entered at the top).

These rules do apply to arrays and structures (termed aggregates); arrays and structures are considered ``variables'' as far as
initialization is concerned. When an automatic array or structure has a partial initializer, the remainder is initialized to 0,
just as for statics.

------------------------------------------------------------------------------------------------------------------------------------
Function calls are allowed in initializers only for automatic variables (that is, for local, non-static variables).

------------------------------------------------------------------------------------------------------------------------------------
char a[] = "string literal";
char *p  = "string literal";

A string literal (the formal term for a double-quoted string in C source) can be used in two slightly different ways:

As the initializer for an array of char, as in the declaration of char a[] , it specifies the initial values of the characters
in that array (and, if necessary, its size).

Anywhere else, it turns into an unnamed, static array of characters, and this unnamed array may be stored in read-only memory, and
which therefore cannot necessarily be modified. In an expression context, the array is converted at once to a pointer, as usual, so
the second declaration initializes p to point to the unnamed array's first element.

------------------------------------------------------------------------------------------------------------------------------------
char a[3] = "abc"; legal? What does it mean?

It declares an array of size three, initialized with the three characters 'a', 'b', and 'c', without the usual terminating '\0'
character. The array is therefore not a true C string and cannot be used with strcpy, printf's %s format, etc.
Most of the time, you should let the compiler count the initializers when initializing arrays (in the case of the initializer "abc",
of course, the computed size will be 4).

------------------------------------------------------------------------------------------------------------------------------------
extern int func();
int (*fp)() = func;
When the name of a function appears in an expression, it ``decays'' into a pointer (that is, it has its address implicitly taken),
much as an array name does.

------------------------------------------------------------------------------------------------------------------------------------
How can I implement opaque (abstract) data types in C?

One good way is for clients to use structure pointers (perhaps additionally hidden behind typedefs) which point to structure types
which are not publicly defined. In other words, a client uses structure pointers (and calls functions accepting and returning
structure pointers) without knowing anything about what the fields of the structure are. (As long as the details of the structure
aren't needed--e.g. as long as the -> and sizeof operators are not used--C is perfectly happy to handle pointers to structures of
incomplete type.

------------------------------------------------------------------------------------------------------------------------------------
Anytime within a program in which you specify a value explicitly instead of referring to a variable or some other form of data,
that value is referred to as a literal.

------------------------------------------------------------------------------------------------------------------------------------
The size of a byte is specified by the macro CHAR_BIT which specifies the number of bits in a char (byte).
In standard C it never can be less than 8 bits.
Note that the char value must be enclosed within single quotations.

------------------------------------------------------------------------------------------------------------------------------------
Note that a Standard conforming compiler must issue a warning if an attempt is made to change a const variable - but after doing so
the compiler is free to ignore the const qualifier.

------------------------------------------------------------------------------------------------------------------------------------
C99 introduces the concept of a flexible array member, which allows the size of an array to be omitted if it is the last member in
a structure, thus providing a well-defined solution.

------------------------------------------------------------------------------------------------------------------------------------
(Note that when a structure is assigned, passed, or returned, the copying is done monolithically.
This means that the copies of any pointer fields will point to the same place as the original. In other words, the data pointed to
is not copied.)

------------------------------------------------------------------------------------------------------------------------------------
Static variables are initialized at compile time.

------------------------------------------------------------------------------------------------------------------------------------
If the dimension is specified, but not all elements in the array are initialized, the remaining elements will contain a value of 0.
This is very useful, especially when we have very large arrays.

int numbers[2000]={245};
The above example sets the first value of the array to 245, and the rest to 0.

------------------------------------------------------------------------------------------------------------------------------------
modulus returns the remainder of a division (e.g. 5 % 2 = 1 )
Modulus is not defined for floating-point numbers, but the math.h library has an fmod function.
It's also not equal to math modulus.

------------------------------------------------------------------------------------------------------------------------------------
float *g(), (*h)();
g is a function returning a pointer to float; h is a pointer to function returning float.

------------------------------------------------------------------------------------------------------------------------------------
Here's an example where the test condition is simply a variable. If the variable has a value of 0 or NULL, the loop exits,
otherwise the statements in the body of the loop are executed.
initialization; test; increment

for (t = list_head; t; t = NextItem(t)) {
   /* body of loop */
 }
 
------------------------------------------------------------------------------------------------------------------------------------
int *() - func returning a pointer to integer

------------------------------------------------------------------------------------------------------------------------------------
A class combines a data (attributes) and procedures (operations) that operate on that data.
Object is an instance of a class.
The object provides both values and operations on these values so object is more powerful than a var.

------------------------------------------------------------------------------------------------------------------------------------
An array and a function references decay to pointers and thery're second class objects. Struct is a first class object.

------------------------------------------------------------------------------------------------------------------------------------
The size of a union is the maximum of the sizes of its individual members.

------------------------------------------------------------------------------------------------------------------------------------
Referencing an increment more than 1 time in the same expression is undefined behaviour (faq 3.1.)
modify the variable twice between sequence points, > behavior is undefined.

------------------------------------------------------------------------------------------------------------------------------------
When you need to ensure the order of subexpression evaluation, you may need to use explicit temporary variables and
separate statements (parenthesis do not necessarily force the order).

------------------------------------------------------------------------------------------------------------------------------------
The comma operator does guarantee left-to-right evaluation, but the commas separating the arguments in a function call
are not comma operators.  The order of evaluation of the arguments to a function call is unspecified.

------------------------------------------------------------------------------------------------------------------------------------
C sequence points:
`i++ * i++` - UB.
`i = i++` - UB.
`a[i] = i++` - UB.
`i++ && i++` - OK.
`i = i + 1` - OK.

------------------------------------------------------------------------------------------------------------------------------------
What's the difference between ++i and i++?

++i adds one to the stored value of i and returns the new, incremented value to the surrounding expression. (faster since it's 1 op)
i++ adds one to i but returns the prior, unincremented value. (slower since it stores copy of old val to return?)
(see initialization; test; increment)

------------------------------------------------------------------------------------------------------------------------------------
The relational operators, such as <, are all binary; they compare two operands and return a true or false (1 or 0) result.

------------------------------------------------------------------------------------------------------------------------------------
<limits.h> INT_MAX

------------------------------------------------------------------------------------------------------------------------------------
while !feof might be used as like strtok_r

------------------------------------------------------------------------------------------------------------------------------------
 *p++ is equivalent to *(p++);
 use (*p)++ (or perhaps ++*p) to increment the value.
 
------------------------------------------------------------------------------------------------------------------------------------
cast operator is a conversion operator; yileds an rvalue which cannot be assigned to (or incremented).

------------------------------------------------------------------------------------------------------------------------------------
args to funcs are ALWAYS passed by value!!! Func is getting local copy of an arg; if you need to change ptr, use ptr to ptr.
Simulation of passing by reference:
	void f(void **);
	double *dp;
	void *vp = dp;
	f(&vp);
	dp = vp;

A void pointer is not the same type as a structure pointer, and on some machines it may have a different size or representation
(which is why these casts are required for correctness).

------------------------------------------------------------------------------------------------------------------------------------
Calling func via pointer: r = (*fp)(); == r = fp();
On some machines, function addresses can be very large, bigger than any data  (void *) pointers.

------------------------------------------------------------------------------------------------------------------------------------
The expression myArray[i] is equivalent to i[myArray].
The first is equivalent to *(myArray+i), and the second is equivalent to *(i+myArray).
These turn out to be the same, since the addition is commutative.

https://godbolt.org/z/v9Mj4Yhttps://godbolt.org/z/v9Mj4Y

------------------------------------------------------------------------------------------------------------------------------------
sizeof operates at compile-time inside the same function where object is defined
(e.g. #define NUM_ELEM(x) (sizeof (x) / sizeof (*(x)))); NUM_ELEM(arr) - only if ARR is in the same function where the sizeof is)
Unfortunately, (in C and C++) the length of the array cannot be obtained from an array passed in at run time, because
(as mentioned above) the size of an array is not stored anywhere. The compiler always replaces sizeof with a constant.

------------------------------------------------------------------------------------------------------------------------------------
char* p1 = "onetwothree";
char* p2 = "twothree";
char* p3 = "three";

-O2/-O3:

p1 p2
v  v
onetwothree\0
      ^
      p3
base address is different, and '\0' is at the end.

------------------------------------------------------------------------------------------------------------------------------------
always use parenthesis for #define's. (and compiler calculates constant expressions).
#define X 2 + 3
printf("%d\n", X * 4);

preprocessor will put '2 + 3 * 4'

------------------------------------------------------------------------------------------------------------------------------------
char *p = malloc(10); - implicit conversion of malloc ret value to char *
char *p = (char *)malloc(10); - explicit (and c++ - required) conversion

------------------------------------------------------------------------------------------------------------------------------------
void f(int *ip) {
static int dummy = 5;
ip = &dummy;
}

int *ip;
f(ip); - f receives a copy of the pointer (and alters a copy)!
as we have pass-by-value. To pass by ref we'd need to accept ptr to ptr:

void f(int **ip)... + f(&ip)
------------------------------------------------------------------------------------------------------------------------------------
Function calls are allowed in initializers only for automatic variables (that is, for local, non-static variables).
e.g. char *p = malloc(10); is OK as p has an automatic storage type. 

------------------------------------------------------------------------------------------------------------------------------------
The malloc/free implementation remembers the size of each block as it is allocated, so it is not necessary to remind it of the size
when freeing. (Typically, the size is stored adjacent to the allocated block.)

------------------------------------------------------------------------------------------------------------------------------------
pointer subtraction is only defined when performed on pointers into the same object.

realloc may not always be able to enlarge memory regions in-place.
When it is able to, it simply gives you back the same pointer you handed it, but if it must go to some other part of memory
to find enough contiguous space, it will return a different pointer (and the previous pointer value will become unusable).
If realloc cannot find enough space at all, it returns a null pointer, and leaves the previous region allocated.

When reallocating memory, be careful if there are any other pointers lying around which point into alias that memory:
if realloc must locate the new region somewhere else, those other pointers must also be adjusted.

------------------------------------------------------------------------------------------------------------------------------------
calloc(m, n) == p = malloc(m * n); memset(p, 0, m * n);

------------------------------------------------------------------------------------------------------------------------------------
memdup() = malloc() + memcpy()

------------------------------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------------------