#if 0
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

void handler(int num)
{
	printf("signal: %d\n", num);
}

int main(int argc, char *argv[])
{
        fd_set rfds;
        struct timeval tv;
	struct sigaction sig;
        int retval;
	char buf[1024];
	sig.sa_handler = handler;
	sigemptyset(&sig.sa_mask);
        sig.sa_flags = SA_RESTART;
	if (argc < 2 || argv[1][0] == 'r') { 
        	if (sigaction(SIGQUIT, &sig, NULL)) {
			perror("sigaction error");
			exit(1);
		}
	}

#if 0
        /* Watch stdin (fd 0) to see when it has input. */
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
	printf("our pid is %d\n", getpid());

        /* Wait up to five seconds. */
	
        	tv.tv_sec = 5;
        	tv.tv_usec = 0;
        	retval = select(1, &rfds, NULL, NULL, &tv);
        	retval = select(1, &rfds, NULL, NULL, NULL);
        	/* Don't rely on the value of tv now! */

        	if (retval == -1 && errno == EINTR)
        	    perror("select()");
        	else if (retval) {
        	    printf("Data is available now.\n");
        	    /* FD_ISSET(0, &rfds) will be true. */
		    read(0, buf, 1024);
		}
        	else
        	    printf("No data within five seconds.\n");
#else
	retval = read(0, buf, 1024);
        if (retval == -1 && errno == EINTR)
            perror("read()");
#endif
	return 0;
}
#else
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int got_interrupt;

void intrup(int dummy) {
        got_interrupt = 1;
	printf("signal num: %d\n", dummy);
}

void die(char *s) {
        printf("%s\n", s);
        exit(1);
}

int main() {
        struct sigaction sa;
        int n;
        char c;
	struct sockaddr_in seraddr;
	seraddr.sin_family = AF_INET;
	seraddr.sin_addr.s_addr = inet_addr("192.168.0.191");
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	
	connect(sock, (struct sockaddr *)&seraddr, sizeof(struct sockaddr));
	

        sa.sa_handler = intrup;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        if (sigaction(SIGQUIT, &sa, NULL))
                die("sigaction-SIGINT");
        sa.sa_flags = SA_RESTART;
        if (sigaction(SIGINT, &sa, NULL))
                die("sigaction-SIGQUIT");

        got_interrupt = 0;
        //n = read(0, &c, 1);
        n = recvfrom(sock, &c, 1, 0, 0, 0);
        if (n == -1 && errno == EINTR)
                printf("read call was interrupted\n");
        else if (got_interrupt)
                printf("read call was restarted\n");

        return 0;
}
#endif
