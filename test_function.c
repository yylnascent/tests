#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>


test_once(const char *ip, const char *port)
{
	int sock;
	struct sockaddr_in addrsev;
	char buf[1024];
	int rc;
	addrsev.sin_family = AF_INET;
	addrsev.sin_port = htons(atoi(port));
	inet_pton(AF_INET, ip, &addrsev.sin_addr);


	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("create socket fail\n");
		exit(1);
	}
	
	rc = connect(sock, (struct sockaddr *)&addrsev, sizeof(struct sockaddr));
	if (rc == -1) {
		perror("connect fail\n");
		exit(1);
	}
	
	do {
		rc = read(sock, buf, 1024);
	} while (rc > 0);

	return;
}
