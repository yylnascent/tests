#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PGM "test_rate_server"


double max(double first, double second)
{
	return first > second ? first : second;
}

double min(double first, double second)
{
	return first < second ? first : second;
}

int time_sub(struct timeval *result, struct timeval first, struct timeval second)
{
	result->tv_sec = first.tv_sec - second.tv_sec;
	result->tv_usec = first.tv_usec - second.tv_usec;
	if (result->tv_usec < 0) {
		result->tv_sec--;
		result->tv_usec += 1000000;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int sock_lis = -1;
	int sock_rev = -1;
	int rc;
	struct sockaddr_in addrin;
	struct sockaddr_in addrout;
	int reuse = 1;
	char buf[1024];
	int index;
	int loop = 1024;
	int port = 55555;
	socklen_t len = sizeof(struct sockaddr);
	int opt;
	struct timeval start;
	struct timeval stop;
	struct timeval once_start;
	struct timeval once_stop;
	struct timeval result;
	int writed;
	int total_writed;
	int interval = 1;
	double maxspeed = 0;
	double minspeed = 1000; // it is so big, that we can not reach!
	double curspeed = 0;

	while ((opt = getopt(argc, argv, "s:p:i:h")) != -1) {
		switch (opt) {
			case 's': loop *= atoi(optarg);
				break;
			case 'p': port = atoi(optarg);
				break;
			case 'i': interval = atoi(optarg);
				break;
			case 'h':
			default:
				printf("usage:"PGM" -h <for help> -s <size of date, default 1(M)> -p <port, default 55555> -i <intervel to display, default 1MB size>\n");
				exit(1);
		}
	}

	signal(SIGPIPE, SIG_IGN);

	memset(&addrin, 0, len); 
	addrin.sin_family = AF_INET;
	addrin.sin_port = htons(port);
	addrin.sin_addr.s_addr = htonl(INADDR_ANY);

	printf("listening port: %d\n", port);

	sock_lis = socket(AF_INET, SOCK_STREAM, 0);

	if (sock_lis == -1) {
		perror("create socket fail");
		exit(1);
	}
	// we do not check result
	reuse = 1;
	setsockopt(sock_lis, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse, sizeof(reuse));
#ifndef ANDROID
	reuse = 1;
	setsockopt(sock_lis, SOL_SOCKET, SO_REUSEPORT, (const void *)&reuse, sizeof(reuse));
#endif

	rc = bind(sock_lis, (const struct sockaddr *)&addrin, len);
	if (rc != 0) {
		perror("bind fail");
		exit(1);
	}

	rc = listen(sock_lis, SOMAXCONN);
	if (rc != 0) {
		perror("listen fail");
		exit(1);
	}

	while (1) {
		memset(&addrout, 0, len);
		sock_rev = accept(sock_lis, (struct sockaddr *)&addrout, &len);
		if (sock_rev == -1) {
			perror("accept fail");
			continue;
		} 
		printf("client in: %s\n", inet_ntoa(addrout.sin_addr));
		writed = 0;
		total_writed = 0;
		index = 0;
		gettimeofday(&start, 0);
		gettimeofday(&once_start, 0);
		while (index++ < loop) {
			rc = write(sock_rev, buf, 1024);
			if (rc == -1) {
				perror("write fail");
				continue;
			}
			writed++;
			total_writed++;
			if (writed == 1024*interval) {
				gettimeofday(&once_stop, 0);
				memset(&result, 0, sizeof(struct timeval));
				time_sub(&result, once_stop, once_start);
				curspeed = (writed*1024.0)/(result.tv_sec*1000 + (result.tv_usec/1000))/1000.0;
				printf("we have send %d MB data, and speed is %.3fMB/s\n", interval, curspeed);
				minspeed = min(curspeed, minspeed);
				maxspeed = max(curspeed, maxspeed);
				gettimeofday(&once_start, 0);
				writed = 0;
			}
		}
		gettimeofday(&stop, 0);
		memset(&result, 0, sizeof(struct timeval));
		time_sub(&result, stop, start);
		printf("we have send %f MB data, ",  total_writed/1024.0);
		printf("and speed is %.3fMB/s\n", (total_writed*1024.0)/(result.tv_sec*1000 + (result.tv_usec/1000))/1000.0);
		printf("the max speed is %f MB/s\n",  maxspeed);
		printf("the min speed is %f MB/s\n",  minspeed);
		fflush(stdout);
		close(sock_rev);
	}	

	return 0;	
}
