#include <string.h>
#include <stdio.h>
#include <assert.h>

static void reverseString(char *str, size_t l) {
	size_t orig_str_len = l;
	char new_str[orig_str_len + 1];

	for (size_t i = 0; i < orig_str_len; ++i) {
		// fprintf(stdout, "new_str[%lu] = %c\n", i, *(str + ( orig_str_len - i )));
		memcpy( (new_str + i), (str + ( orig_str_len - i - 1)), 1);
		new_str[orig_str_len] = '\0';
	}

	// *str = *new_str; // str[0] = new_str[0]
	memcpy( str, new_str, strlen(new_str) );
	// fprintf(stdout, "%s\n", new_str);
}


int main(void) {
	char string[] = "ABCDEF";
	reverseString(string, l);
	return 0;
}
