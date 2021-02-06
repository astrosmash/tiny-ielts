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

char * reverseVowels(char * s){

    const char vowels[] = { 'a', 'A', 'e', 'E', 'i', 'I', 'o', 'O', 'u', 'U' }; 
    // size_t orig_str_len = sizeof(s) / sizeof(char);
    size_t orig_str_len = strlen(s);
    char new_str[orig_str_len + 1];
    fprintf(stdout, "%llu\n", orig_str_len);

    for (int i = 0; i < orig_str_len; ++i) {

        for (int j = 0; j < 10; ++j) {
		if (s[i] == vowels[j]) {
			fprintf(stdout, "vow detected %c\n\n", s[i]);
			new_str[orig_str_len - i] = s[i];
			continue;
		}
	}
        new_str[i] = s[i];

    }

    fprintf(stdout, "%s\n", new_str);
}

int main(void) {
	char string[] = "ABsdasdkjfqwefDSUISDEF";
	//reverseString(string, strlen(string));
	reverseVowels(string);
	return 0;
}
