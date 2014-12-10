#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <netinet/if_ether.h>
#define INLEN 4
#define MAC_BCAST_ADDR  (uint8_t *) "\xff\xff\xff\xff\xff\xff"
void usage_quit(char *arg0);
int get_ifi(char *dev, char *mac, int macln, struct in_addr *lc_addr, int ipln);
void prmac(u_char *ptr);
int main(int argc, char **argv)
{
        if(argc != 2) {
                usage_quit(argv[0]);
	}
        int reqfd, recvfd, salen, n;  
	struct timeval tv_out;
        u_char *mac;
        char recv_buf[120], rep_addr[16];
        struct in_addr lc_addr, req_addr;
        struct sockaddr_ll reqsa, repsa;
        struct arp_pkt {
                struct ether_header eh;
                struct ether_arp ea;
                u_char padding[18];
        } req;
       
        bzero(&reqsa, sizeof(reqsa));
        reqsa.sll_family = PF_PACKET;
        reqsa.sll_ifindex = if_nametoindex("eth1");
        if((reqfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_RARP))) < 0) {
                perror("Socket error");
                exit(1);
        }
        mac = (char *)malloc(ETH_ALEN);
        bzero(&req, sizeof(req));
        if(get_ifi("eth1", mac, ETH_ALEN, &lc_addr, INLEN)) {
                fprintf(stderr, "Error: Get host’s information failed\n");
                exit(0);
        }
       
        memcpy(req.eh.ether_dhost, MAC_BCAST_ADDR, ETH_ALEN);
        memcpy(req.eh.ether_shost, mac, ETH_ALEN);
        req.eh.ether_type = htons(ETHERTYPE_ARP);
       
        req.ea.arp_hrd = htons(ARPHRD_ETHER);
        req.ea.arp_pro = htons(ETHERTYPE_IP);
        req.ea.arp_hln = ETH_ALEN;
        req.ea.arp_pln = INLEN;
        req.ea.arp_op = htons(ARPOP_REQUEST);
        memcpy(req.ea.arp_sha, mac, ETH_ALEN);
        memcpy(req.ea.arp_spa, &lc_addr, INLEN);
        inet_aton(argv[1], req.ea.arp_tpa);
        if((n = sendto(reqfd, &req, sizeof(req), 0, (struct sockaddr *)&reqsa, sizeof(reqsa))) <= 0) {
                perror("Sendto error");
                exit(1);
        }
        printf("Broadcast arp request of %s, %d bytes be sent\n\n", argv[1], n);
        recvfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
        bzero(recv_buf, sizeof(recv_buf));
        bzero(&repsa, sizeof(repsa));
        salen = sizeof(struct sockaddr_ll);
	tv_out.tv_sec = 0;
	tv_out.tv_usec = 100 * 1000;
	setsockopt(recvfd, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));
        while(1) {
                if((n = recvfrom(recvfd, recv_buf, sizeof(req), 0, (struct sockaddr *)&repsa, &salen)) <= 0) {
                        perror("Recvfrom error");
                        exit(1);
                }
                if( ntohs(*(__be16 *)(recv_buf + 20))==2 && !memcmp(req.ea.arp_tpa, recv_buf + 28, 4) ) {
                        printf("Response from %s, %d bytes received\n", argv[1], n);
                        printf("        Peer IP is: %s\n", (const char *)inet_ntop(AF_INET, (const void *)(recv_buf + 28), rep_addr, 1024));
                        prmac( (u_char *)(recv_buf + 22) ); //prmac( (u_char *)(recv_buf + 6) );
                        break;
                }
        }
        free(mac);
}

int get_ifi(char *dev, char * mac, int macln, struct in_addr *lc_addr, int ipln)
{
        int reqfd, n;
        struct ifreq macreq;
        reqfd = socket(AF_INET, SOCK_DGRAM, 0);
        strcpy(macreq.ifr_name, dev);
       
        if(ioctl(reqfd, SIOCGIFHWADDR, &macreq) != 0)
                return 1;
        memcpy(mac, macreq.ifr_hwaddr.sa_data, macln);
       
        if(ioctl(reqfd, SIOCGIFADDR, &macreq) != 0)
                return 1;
        memcpy(lc_addr, &((struct sockaddr_in *)(&macreq.ifr_addr))->sin_addr, ipln);
        return 0;
}      
void prmac(u_char *ptr)
{
	char buff[18] = {0};
	sprintf(buff, "%02x:%02x:%02x:%02x:%02x:%02x", *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5));
        printf("        Peer MAC is: %s\n", buff);
}
void usage_quit(char *arg0)
{
        fprintf(stderr, "Usage: %s <query_IP>\n", arg0);
        exit(0);
}
