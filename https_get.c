#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PGM "https_get"

void usage(void)
{
	fprintf(stderr, "%s -h <help display help>, -a <remote host ip, default 127.0.0.1> -r <requestpath, default '/' must start with '/'> \
 -v <verify, default not verify>\n", PGM);
}

int callback(X509_STORE_CTX *ctx, void *omit)
{
	int ret;
	(void)omit;
	ret = X509_verify_cert(ctx);
	if (ret != 1) {
		fprintf(stderr, "verify cer err.error= %d, info: %s\n",ctx->error,X509_verify_cert_error_string(ctx->error));
	}
	return ret;
}


int main(int argc, char *argv[])
{
	SSL *ssl;
	SSL_CTX *ctx;
	struct sockaddr_in saddr;
	int ret;
	int sock;
	char buf[1024];
	char revbuf[1024];
	int verify = 0;
	char *requestpath = "/";
	char *host = "127.0.0.1";
	FILE *fp;
	int opt;

	while ((opt = getopt(argc, argv, "r:a:vh")) != -1) {
		switch (opt) {
			case 'a': host = optarg; break;
			case 'r': requestpath = optarg; break;
			case 'v': verify = 1; break;
			case 'h': 
			default : usage(); exit(1);
		}
	}

	fp = fopen("index.html", "w+");
	if (fp == NULL) {
		perror("create file");
		exit(1);
	}

	memset(buf, 0, 1024);
	sprintf(buf, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", requestpath, host); 
	// connect to server
	memset(&saddr, 0, sizeof(struct sockaddr_in));
	saddr.sin_family = AF_INET;
	inet_pton(AF_INET, host, &saddr.sin_addr);
	saddr.sin_port = htons(443);
	fprintf(stderr, "starting...\n");
	fflush(NULL);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("create");
		exit(1);
	}

	ret = connect(sock, (const struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
	if (ret == -1) {
		perror("connect");
		goto cleanfd;
	}
	fprintf(stderr, "connect  successfully\n");

	SSL_load_error_strings();
	SSL_library_init();
	ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL) {
		fprintf(stderr, "init SSL CTX failed: %s\n", ERR_reason_error_string(ERR_get_error()));
		goto cleanfd;
	}

	if (verify) {
		SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
		SSL_CTX_set_cert_verify_callback(ctx, callback, 0);
		SSL_CTX_load_verify_locations(ctx, NULL, "/etc/ssl/certs");
	}

	ssl = SSL_new(ctx);
	if (ssl == NULL) {
		fprintf(stderr, "new ssl with created ctx failed: %s\n", ERR_reason_error_string(ERR_get_error()));
		goto cleanctx;
	}
	
	ret = SSL_set_fd(ssl, sock);
	if (ret == 0) {
		fprintf(stderr, "add SSL to tcp connection failed: %s\n", ERR_reason_error_string(ERR_get_error()));
		goto cleanssl;
	}
	
	ret = SSL_connect(ssl);
	if (ret != 1) {
		fprintf(stderr, "SSL connect failed: %s\n", ERR_reason_error_string(ERR_get_error()));
		goto cleanssl;
	}
	fprintf(stderr, "SSL connect successfully\n");

	SSL_write(ssl, buf, strlen(buf));
	memset(revbuf, 0, 1024);
	while ((ret = SSL_read(ssl, revbuf, sizeof(revbuf) - 1)) > 0) {
		fprintf(stderr, "%s", revbuf);
		fprintf(fp, "%s", revbuf);
		memset(revbuf, 0, 1024);
	}

	ret = SSL_shutdown(ssl);
	if (ret == -1) {
		fprintf(stderr, "SSL shutdown failed: %s\n", ERR_reason_error_string(ERR_get_error()));
		goto cleanssl;
	}

cleanssl:
	fclose(fp);
	SSL_free(ssl);
cleanctx:
	SSL_CTX_free(ctx);
cleanfd:
	close(sock);

	return 0;
}
