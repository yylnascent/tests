#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>


int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in saddr;
	char header[1024];
	char soap[1024];
	char result[1024];
	int rc;
	
	if (argc < 2) {
		printf("we need server ip\n");
		exit(1);
	}
	memset(soap, 0, 1024);
	sprintf(soap, "%s", "<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope xmlns:ns0=\"urn:schemas-upnp-org:service:ContentDirectory:1\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body><ns0:Browse><ObjectID>0</ObjectID><BrowseFlag>BrowseDirectChildren</BrowseFlag><Filter>*</Filter><StartingIndex>0</StartingIndex><RequestedCount>0</RequestedCount><SortCriteria /></ns0:Browse></s:Body></s:Envelope>");
	memset(header, 0, 1024);
	sprintf(header, "POST /control/cds_control HTTP/1.0\nHost: %s\nUser-Agent: Coherence PageGetter\nContent-Length: %d\nSOAPACTION: \"urn:schemas-upnp-org:service:ContentDirectory:1#Browse\"\ncontent-type: text/xml ;charset=\"utf-8\"\nconnection: close\n\n", argv[1], strlen(soap)); 

	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = inet_addr(argv[1]);
	saddr.sin_port = htons(49200);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	
	connect(sock, (struct sockaddr *)&saddr, sizeof(struct sockaddr));
	
	write(sock, header, strlen(header));

	write(sock, soap, strlen(soap));
	memset(result, 0, 1024);
	while ((rc = read(sock, result, 1024)) != 0) {
		printf("%s", result);
		memset(result, 0, 1024);
	}

	printf("\n");
	return 0;
}
