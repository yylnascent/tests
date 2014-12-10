#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#define PGM "test_rate_client"

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in addrsev;
	char buf[1024];
	int rc;
	int opt;
	char *ip = NULL;
	int port = 55555;
	uint64_t rev = 0;

	while ((opt = getopt(argc, argv, "r:p:h")) != -1) {
		switch (opt) {
			case 'r': ip = optarg;
				break;
			case 'p': port = atoi(optarg);
				break;
			case 'h':
			default:
				printf("usage:"PGM" -h <for help> -r <ip to connect> -p <port>\n");
				exit(1);
		}
	}

	if (!ip) {
		printf("we need a ip to connect\n");
		exit(1);
	}
	addrsev.sin_family = AF_INET;
	addrsev.sin_port = htons(port);
	inet_pton(AF_INET, ip, &addrsev.sin_addr);


	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("create socket fail");
		exit(1);
	}
	
	rc = connect(sock, (struct sockaddr *)&addrsev, sizeof(struct sockaddr));
	if (rc == -1) {
		perror("connect fail");
		exit(1);
	}
	
	do {
		rc = read(sock, buf, 1024);
		rev += rc;
	} while (rc > 0);
	
	printf("Totally, we receive %.3fMB\n", rev/1024.0/1024);

	return 0;
}
