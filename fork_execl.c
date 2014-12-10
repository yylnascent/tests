#include <stdio.h>
#include <unistd.h>

int main()
{
	int pid = fork();
	if (pid > 0) {
		int status;
       		pid_t tmpPid = wait(&status);
       		if (tmpPid == pid)
       		{
       		    printf("父进程通过 wait 函数知道子进程已经结束\n");
       		}
	} else if (0 == pid) {
		execl("sendrcvarp", "sudo ./sendrcvarp", "192.168.0.13", NULL);
	} else {
		return 0;
	}
}
