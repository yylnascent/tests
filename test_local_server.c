#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/un.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define server "local_server"

void die(const char *errlog)
{
	perror(errlog);
	exit(1);
}


int main(int argc, char *argv[])
{
	int listensock;
	int accsock;
	int opt;
	int ret;
	struct sockaddr_un localaddr;
	struct sockaddr_un peeraddr;
	//char wd[PATH_MAX];
	//char curfile[PATH_MAX];
	char buf[1024];
	
	//memset(wd, 0, PATH_MAX);
	//memset(curfile, 0, PATH_MAX);
	//getcwd(wd, PATH_MAX);
	//sprintf(curfile, "%s/%s", wd, server);
	//printf("pwd: %s\n", curfile);
	//unlink(curfile);
	unlink(server);
	memset(&localaddr, 0, sizeof(struct sockaddr_un));
	localaddr.sun_family = AF_UNIX;
	//strncpy(&localaddr.sun_path, curfile, strlen(curfile));
	strncpy(&localaddr.sun_path, server, strlen(server));
	
	listensock = socket(AF_UNIX, SOCK_STREAM, 0);
	
	if (listensock == -1) {
		die("create socket");
	}


	ret = bind(listensock, (const struct sockaddr *)&localaddr, sizeof(struct sockaddr_un));
	if (ret == -1) {
		die("bind");
	}

	ret = listen(listensock, 10);
	if (ret == -1) {
		die("accept");
	}


	while (accsock = accept(listensock, (struct sockaddr *)&peeraddr, sizeof(struct sockaddr_un))) {
		if (accsock == -1) {
			continue;
		}
		memset(buf, 0, 1024);
		while ((ret = recv(accsock, buf, 1023, 0)) > 0) {
			sprintf(stderr, "%s", buf);
			memset(buf, 0, 1023);
		}
		close(accsock);
	}

	close(listensock);

	return 0;
}
