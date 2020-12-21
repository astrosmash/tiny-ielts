#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// void reverseString(char* s, int sSize){
// 		char* result = NULL;
// 		result = (char*)malloc(sSize * sizeof(char));
// 		if (!result) return;
//
// 		// fprintf(stdout, "%s", sizeof(result));
// 		//
//     // for (int i = 0; i < sSize; ++i) {
// 		// 	char needed = s[sSize - i];
// 		// 	fprintf(stdout, "%s", result);
// 		// 	result[i] = needed;
//     // }
// 		//
// 		// 	fprintf(stdout, "%s\n", result);
// 		memset(&s, 0, 8);
// 		result = "sdsd";
//     s = result;
// }



void reverseString(char* s, int sSize){

		char* rev_str = (char*)malloc(sSize * sizeof(char));
		if (!rev_str) return;

		// rev_str = "SDDSSDSDDSSDSDDSSDSDDSSDSDDSSDSDDSSD";

		// memset(&str, 0, sizeof(str));
		// fprintf(stdout, "%s\n", s);

    // for (int i = 0; i < sSize; ++i) {
    //     if ((s + i) == NULL) return;
    //     if ((str + i) == NULL) return;
		// 		 strcpy(s, str);
    // }

    // *s = *rev_str; // Single char is dereferenced and assigned, to assign a whole str, we need to loop per-char (strlen, not sizeof)
		free(rev_str);
}





int main(void) {
	char* my_str = (char*)malloc(100 * sizeof(char));
	if (my_str != NULL) {

		// my_str = "sdjkdjsHDUUIDJSHJKhjk91";
		// reverseString(my_str, strlen (my_str));
		// fprintf(stdout, "%s\n", my_str);
		free(my_str);
	}
	return 0;
}
