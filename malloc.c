#include <stdlib.h>
#include <stdio.h>

int main()
{
	int *p = NULL;
	p = (int *)malloc(sizeof(int));
	p = (int *)malloc(sizeof(int));
	p = (int *)malloc(sizeof(int));
	p = (int *)malloc(sizeof(int));
	p = (int *)malloc(sizeof(int));
	p = (int *)malloc(sizeof(int));
	p = (int *)malloc(sizeof(int));
	p = (int *)malloc(sizeof(int));
	p = (int *)malloc(sizeof(int));
	p = (int *)malloc(sizeof(int));
	*p = 100;
	printf("*p = %d\n", *p);
	return 0;
	//free(p);
}
