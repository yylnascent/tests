#include <pcap.h>
#include <stdio.h>
#include <libnet.h>
#include <string>
#include <unistd.h>

using namespace std;

typedef struct arphdr{
unsigned short htype; //hardware type
unsigned short ptype; //protocol type
unsigned char hlen; //hardware address length
unsigned char plen; //protocol address length
unsigned short oper; //operation code
unsigned char sha[6]; //sender hardware address
unsigned char spa[4]; //sender ip address
unsigned char tha[6]; //target hardware address
unsigned char tpa[4]; //target ip address
}arphdr_t;


int GetIfaceAddr(const char *iface, char *addr)
{

	if (!iface) {
		return -1;
	}

	int sock = socket (AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		printf("socket error");
		return -1;
	}

	struct ifreq ifr;
	strcpy (ifr.ifr_name, iface);
	ifr.ifr_addr.sa_family = AF_INET;

	if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
		printf("ioctl error");
		close (sock);
		return -1;
	}

	close (sock);

	const char *rc = inet_ntop(AF_INET, &((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr, addr, INET_ADDRSTRLEN);

	return rc ? 0 : -1;
}

int GetIfHwAddr(const char *iface, u_int8_t addr[])
{

	if (!iface) {
		return -1;
	}

	int sock = socket (AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		printf("socket error");
		return -1;
	}

	struct ifreq ifr;
	strcpy (ifr.ifr_name, iface);
	ifr.ifr_addr.sa_family = AF_INET;

	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
		printf("ioctl error");
		close (sock);
		return -1;
	}

	close (sock);
	
	addr[0] = (unsigned char)ifr.ifr_hwaddr.sa_data[0];
	addr[1] = (unsigned char)ifr.ifr_hwaddr.sa_data[1];
	addr[2] = (unsigned char)ifr.ifr_hwaddr.sa_data[2];
	addr[3] = (unsigned char)ifr.ifr_hwaddr.sa_data[3];
	addr[4] = (unsigned char)ifr.ifr_hwaddr.sa_data[4];
	addr[5] = (unsigned char)ifr.ifr_hwaddr.sa_data[5];

	return 0;
}

// capature arp packet
int catcharp(const char *, const char *);

// analy arp packet
void process_packet(u_char * args, const struct pcap_pkthdr * header, const u_char * packet)
{
    arphdr_t *arpheader = NULL;
    arpheader = (struct arphdr *)(packet + 14);
    char src_ip[16];
    char dst_ip[16];
    char src_hw[18];
    char dst_hw[18];
    sprintf(src_ip, "%d.%d.%d.%d", arpheader->spa[0], arpheader->spa[1], arpheader->spa[2], arpheader->spa[3]);
    sprintf(dst_ip, "%d.%d.%d.%d", arpheader->tpa[0], arpheader->tpa[1], arpheader->tpa[2], arpheader->tpa[3]);
    sprintf(src_hw, "%02x:%02x:%02x:%02x:%02x:%02x", arpheader->sha[0], arpheader->sha[1], arpheader->sha[2], arpheader->sha[3], arpheader->sha[4], arpheader->sha[5]);
    sprintf(dst_hw, "%02x:%02x:%02x:%02x:%02x:%02x", arpheader->tha[0], arpheader->tha[1], arpheader->tha[2], arpheader->tha[3], arpheader->tha[4], arpheader->tha[5]);
    printf("%s(%s) -> %s(%s)\n", src_ip, src_hw, dst_ip, dst_hw);
}

int main(int argv, char **argc) 
{
    libnet_t *handle;        /* Libnet句柄 */
    int packet_size;
    if (argv < 3) {
	printf("we need interface and dst_ip.\n");
	return 0;
    }
    char *dst_ip_str = argc[2];
    char *device = argc[1];   
    u_int8_t src_mac[6] = {0x00, 0xa0, 0xb0, 0xa4, 0x01, 0xad};/* 源MAC */
    u_int8_t dst_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};/* 目的MAC,广播地址 */
    u_int8_t rev_mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    u_int32_t dst_ip, src_ip;              /* 网路序的目的IP和源IP */
    char error[LIBNET_ERRBUF_SIZE];        /* 出错信息 */
    libnet_ptag_t arp_proto_tag, eth_proto_tag;
    u_int8_t src_ip_str[32];       /* 源IP地址字符串 */
    
    GetIfaceAddr(device, (char *)src_ip_str);
    GetIfHwAddr(device, src_mac);

    dst_ip = libnet_name2addr4(handle, (char *)dst_ip_str, LIBNET_RESOLVE);
    src_ip = libnet_name2addr4(handle, (char *)src_ip_str, LIBNET_RESOLVE);

    if ( dst_ip == -1 || src_ip == -1 ) {
        printf("ip address convert error\n");
        exit(-1);
    };
    if ( (handle = libnet_init(LIBNET_LINK_ADV, device, error)) == NULL ) {
        printf("libnet_init: error [%s]\n", error);
        exit(-2);
    };

    arp_proto_tag = libnet_build_arp(
                ARPHRD_ETHER,        /* 硬件类型,1表示以太网硬件地址 */ 
                ETHERTYPE_IP,        /* 0x0800表示询问IP地址 */ 
                6,                   /* 硬件地址长度 */ 
                4,                   /* IP地址长度 */ 
                ARPOP_REQUEST,       /* 操作方式:ARP请求 */ 
                src_mac,             /* source MAC addr */ 
                (u_int8_t *)&src_ip, /* src proto addr */ 
                rev_mac,             /* dst MAC addr */ 
                (u_int8_t *)&dst_ip, /* dst IP addr */ 
                NULL,                /* no payload */ 
                0,                   /* payload length */ 
                handle,              /* libnet tag */ 
                0                    /* Create new one */
    );
    if (arp_proto_tag == -1)    {
        printf("build IP failure\n");
        exit(-3);
    };

    /* 构造一个以太网协议块
    You should only use this function when 
    libnet is initialized with the LIBNET_LINK interface.*/
    eth_proto_tag = libnet_build_ethernet(
        dst_mac,         /* 以太网目的地址 */
        src_mac,         /* 以太网源地址 */
        ETHERTYPE_ARP,   /* 以太网上层协议类型，此时为ARP请求 */
        NULL,            /* 负载，这里为空 */ 
        0,               /* 负载大小 */
        handle,          /* Libnet句柄 */
        0                /* 协议块标记，0表示构造一个新的 */ 
    );
    if (eth_proto_tag == -1) {
        printf("build eth_header failure\n");
        return (-4);
    };
    int repeat = 5;
    //int pid = fork();
    //if (0 < pid) {
	char buff[30];
	sprintf(buff, "arp -d %s", dst_ip_str);
	system(buff);
	usleep(1000 * 100);
        while (true) {
    		packet_size = libnet_write(handle);    /* 发送已经构造的数据包*/
		--repeat;
		if (0 == repeat) {
		    break;
		}
	}
   // } else if (0 == pid) {
        catcharp(device, dst_ip_str);
   // } else {
//	return 0;
  //  }

    libnet_destroy(handle);                /* 释放句柄 */

    return (0);
}

int catcharp(const char *dev, const char *src_ip)
{
    pcap_t * handle; // Sesion handle
    char errbuf[PCAP_ERRBUF_SIZE]; // error string 
    char filter_exp[30];
    bpf_u_int32 mask;  //The netmask of our sniffing device 
    bpf_u_int32 net;   //The IP of our sniffing device 
    struct bpf_program fp;  //the compiled filter expression    //查询device的mask和ip       
    sprintf(filter_exp, "arp src %s", src_ip);
    if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1){  
        fprintf(stderr, "Can't get netmask for device %s\n", dev); 
        net = 0;
        mask = 0;
    }      //obtaining packet capture descriptor  
    handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if(handle == NULL){  
        fprintf(stderr, "Can't open device %s\n", dev); 
        return 2; 
    }     // before apply filter exp, compile it   
    if(pcap_compile(handle, &fp, filter_exp, 0, net) == -1){  
        fprintf(stderr, "can't parse filter %s: %s\n", filter_exp, pcap_geterr(handle)); 
        return 2; 
    }      //apply filter to this session  
    if(pcap_setfilter(handle, &fp) == -1){  
        fprintf(stderr, "can't install filter %s: %s\n", filter_exp, pcap_geterr(handle)); 
        return 2; 
    } 
   // now the device is prepared to sniff under the filter condition  
#if 1
    struct pcap_pkthdr *header; // packet header struct 
    const u_char *pkt_data;
    int capcount = 5;
    int capret = 1;
    while (true) {
        if ( capcount <= 0) {
        	break;
        }
        capret = pcap_next_ex(handle, &header, &pkt_data);
        if (1 != capret) {
        	--capcount;
        	switch (capret) {
        		case 0:
        			printf("timeout\n");
        			break;
        		case -1:
        			printf("error: %s\n", pcap_geterr(handle));
        			break;
        		case -2:
        			printf("read from savefile and no more record ro read\n");
        			break;
        		default:
        			printf("unknown error\n");
        	}
        	continue;
        }
	process_packet(0, header, pkt_data);
        --capcount;
    }
#else
    pcap_loop(handle, 5, process_packet, NULL);
#endif

    struct pcap_stat stat;
    pcap_stats(handle, &stat);
    printf("recv %d, drop %d. \n", stat.ps_recv, stat.ps_drop);
    pcap_close(handle); 
}
