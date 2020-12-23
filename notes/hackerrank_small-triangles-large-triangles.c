#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct triangle
{
	int a;
	int b;
	int c;
};

typedef struct triangle triangle;

#include <assert.h>
#include <string.h>
void sort_by_area(triangle *tr, int n) {
    /**
    * Sort an array a of the length n
    */

    assert(tr);
    assert(1 <= n && n <= 100);

    triangle result[n];
    unsigned int weight[n];

    for (int i = 0; i < n; ++i) {
        assert(tr + i);
        triangle *ctr = tr + i;

        assert(1 <= ctr->a && 1 <= ctr->b && 1 <= ctr->c);
        assert(70 >= ctr->a && 70 >= ctr->b && 70 >= ctr->c);
        assert(ctr->a + ctr->b > ctr->c && ctr->a + ctr->c > ctr->b && ctr->b + ctr->c > ctr->a);
        // fprintf(stdout, "asserted ok. %u %u %u\n", ctr->a, ctr->b, ctr->c);

        const unsigned int hot = (ctr->a + ctr->b + ctr->c) / 2;
        const double sqrtable = hot * (hot - ctr->a) * (hot - ctr->b) * (hot - ctr->c);
        const unsigned int area = sqrt(sqrtable);

        result[i] = *ctr;
        weight[i] = area;

        for (int j = 0; j < i; ++j) {
            if ( weight[j] > weight[i] ) {
                triangle temp = result[j];
                // fprintf(stdout, "temp permut. %u %u %u %u\n", j, temp.a, temp.b, temp.c);

                unsigned int tempw = weight[j];
                // fprintf(stdout, "PREV permut. %u(%u) < %u(%u)\n", weight[i], i, weight[j], j);

                result[j] = result[i];
                weight[j] = weight[i];
                result[i] = temp;
                weight[i] = tempw;
                // fprintf(stdout, "permut. %u %u %u %u\n", j, result[j].a, result[j].b, result[j].c);
                // break;
            }
        }

    }
    memcpy(tr, result, n * sizeof(triangle)); 
    // fprintf(stdout, "finished memcpy\n");
}


int main()
{
	int n;
	scanf("%d", &n);
	triangle *tr = malloc(n * sizeof(triangle));
	for (int i = 0; i < n; i++) {
		scanf("%d%d%d", &tr[i].a, &tr[i].b, &tr[i].c);
	}
	sort_by_area(tr, n);
	for (int i = 0; i < n; i++) {
		printf("%d %d %d\n", tr[i].a, tr[i].b, tr[i].c);
	}
	return 0;
}
