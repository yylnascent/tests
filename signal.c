#include <signal.h>
#include <stdio.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


static int wlan = 0;
static int eth = 0;
static int quit = 0;


int GetIfaceAddr(const char *iface, char *addr)
{
	assert(iface);

	if (!iface) {
		return -1;
	}

	int sock = socket (AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		printf("socket. errno: %d\n", errno);
		return errno;
	}

	struct ifreq ifr;
	strcpy (ifr.ifr_name, iface);
	ifr.ifr_addr.sa_family = AF_INET;

	if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
		printf("ioctl. errno: %d\n", errno);
		close (sock);
		return errno;
	}

	close (sock);

	const char *rc = inet_ntop(AF_INET, &((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr, addr, INET_ADDRSTRLEN);

	return rc ? 0 : -1;
}

int IfaceAvail(const char *iface)
{
	assert(iface);

	if (!iface) {
		return -1;
	}

	struct ifaddrs *itflist;
	if (getifaddrs (&itflist) < 0) {
		printf("getifaddrs, errno: %d\n", errno);
		return errno;
	}

	struct ifaddrs *itf = itflist;
	while (itf) {
    		if ((itf->ifa_flags & IFF_UP) && (itf->ifa_flags & IFF_RUNNING) && !strncmp (itf->ifa_name, iface, IFNAMSIZ)) {
			printf("Interface %s is up.\n", iface);
			freeifaddrs (itflist);
			return 0;
		}
		itf = itf->ifa_next;
	}

	freeifaddrs (itflist);

  	printf("Can't find interface %s.\n", iface);

	
	return -1;
}

void process(int num)
{
	printf("signal number: %d\n", num);
	if (num == SIGUSR1) {
		wlan = 1;
		return;
	}

	if (num == SIGUSR2) {
		eth = 1;
		return;
	}

	if (num == SIGRTMIN) {
		quit = 1;
		return;
	}
	return;
}


void main(void)
{
	signal(SIGRTMIN, process);
	signal(SIGUSR1, process);
	signal(SIGUSR2, process);
	signal(SIGPIPE, SIG_IGN);
	char ip[20];

get_wlan:
	if (0 == IfaceAvail("wlan1")) {
		memset(ip, 0, 20);
		if (0 != GetIfaceAddr("wlan1", ip)) {
			goto get_eth;
		}
		printf("now, ip is %s\n", ip);
		goto run_server;
	}

get_eth:
	if (0 == IfaceAvail("eth1")) {
		memset(ip, 0, 20);
		if (0 != GetIfaceAddr("eth1", ip)) {
			goto get_eth;
		}
		printf("now, ip is %s\n", ip);
	}

run_server:
	while (1) {
		if (eth) {
			eth = 0;
			goto get_eth;
		}

		if (wlan) {
			wlan = 0;
			goto get_wlan;
		}

		if (quit) {
			quit = 1;
			goto exit_server;
		}
		usleep(1000 * 1000);
	}

exit_server:
	return;
}
