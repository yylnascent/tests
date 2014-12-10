#include <pthread.h>
#include <stdio.h>

static int retvalue = 0;
static int run = 1;

void ThreadWork(int *running)
{
	int i = 0;
	printf("child thread enter\n");
	while (*running) {
		printf("child thread, The %dth loop\n", i++);
		usleep(1);
	}
	printf("child thread exit\n");
	retvalue = 8;
	pthread_exit((void *)&retvalue);	
}

int main()
{
	pthread_t pt;
	int ret = -1;
	int times = 3;
	int i = 0;
	int *ret_join = NULL;
	ret = pthread_create(&pt, NULL, (void *)ThreadWork, &run);
	if (0 != ret) {
		printf("create child thread failed\n");
		return 1;
	}

	usleep(1);

	for(;i < times; i++) {
		printf("main thread, The %dth loop\n", i);
		usleep(1);
	}

	run = 0;

	pthread_join(pt, (void *)&ret_join);
	printf("child thread return vlaue is %d\n", *ret_join);
	
	return 0;
}
