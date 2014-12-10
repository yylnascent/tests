#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <linux/un.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>

#define server "local_server"

void die(const char *errlog)
{
	perror(errlog);
	exit(1);
}


int main(void)
{
	int sock;
	int ret;
	//char wd[PATH_MAX];
	//char curfile[PATH_MAX];
	const char *msg = "Hello World!";
	struct sockaddr_un remoteaddr, localaddr;
	char buf[1024];

	memset(&remoteaddr, 0, sizeof(struct sockaddr_un));
	memset(&localaddr, 0, sizeof(struct sockaddr_un));
	localaddr.sun_family = AF_UNIX;
	strncpy((char *)&localaddr.sun_path, "local_client", strlen("local_client"));
	remoteaddr.sun_family = AF_UNIX;
	//memset(wd, 0, PATH_MAX);
	//memset(curfile, 0, PATH_MAX);
	//getcwd(wd, PATH_MAX);
	//sprintf(curfile, "%s/%s", wd, server);
	//printf("server: %s\n", curfile);
	//strncpy(remoteaddr.sun_path, curfile, strlen(curfile));
	strncpy((char *)&remoteaddr.sun_path, server, strlen(server));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);

	unlink("local_client");
	
	ret = bind(sock, (const struct sockaddr *)&localaddr, sizeof(struct sockaddr_un));
	if (ret == -1) {
		die("connect");
	}

	
	ret = connect(sock, (const struct sockaddr *)&remoteaddr, sizeof(struct sockaddr_un));
	if (ret == -1) {
		die("connect");
	}

	ret = write(sock, msg, strlen(msg));
	if (ret == -1) {
		die("write");
	}

	memset(buf, 0, 1024);
	while ((ret = read(sock, buf, 1023)) > 0) {
		sprintf(stderr, "%s", buf);
		memset(buf, 0, 1024);
	}

	close(sock);
	
	return 0;
}
