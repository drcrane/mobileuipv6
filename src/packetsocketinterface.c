#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#include <sys/ioctl.h>
//#include <net/if.h>
#include "packetsocketinterface.h"

#include <errno.h>

#undef ETH_FRAME_LEN
#define ETH_FRAME_LEN 1518

static char mymac[6];
static char gwmac[6];
static char clmac[6];

static int myifindex;

char mm[] = {0x00,0x14,0xa5,0xef,0xdb,0x65};
char dm[] = {0x74,0xe5,0x0b,0x28,0x7a,0x56};

int rawframe_getRawSocket(char * ifname) {
	int s;
	int bindres;
	struct ifreq eth;
	struct sockaddr_ll socket_address;

	memcpy(mymac, mm, 6);
	memcpy(gwmac, dm, 6);

	/* actually open socket */
	s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	fprintf(stdout, "Socket handle: %d\n", s);

	if (s < 0) { return -1; }

	/* enable promisc mode */
	strcpy(eth.ifr_name, ifname);
	ioctl(s, SIOCGIFFLAGS, &eth);
	eth.ifr_flags |= IFF_PROMISC;
	ioctl(s, SIOCSIFFLAGS, &eth);

	/*
	 * get index... probably come in handy later
	 * for example binding and sending maybe??
	 */
	memset(&eth, 0, sizeof(struct ifreq));
	strcpy(eth.ifr_name, ifname);
	ioctl(s, SIOCGIFINDEX, &eth);
	myifindex = eth.ifr_ifindex;

	//memset(&socket_address, 0, sizeof(socket_address));
	//socket_address.sll_family = PF_PACKET;
	//socket_address.sll_protocol = htons(ETH_P_ALL);
	//socket_address.sll_ifindex  = myifindex;
	//socket_address.sll_hatype = ARPHRD_PPP;
	//socket_address.sll_hatype = ARPHRD_ETHER;
	//socket_address.sll_pkttype = PACKET_OTHERHOST;
	//socket_address.sll_halen = ETH_ALEN;
	//bindres = bind(s, (struct sockaddr *)&socket_address, sizeof(struct sockaddr_ll));

	fprintf(stdout, "bind(): %d\n", bindres);
	fprintf(stdout, "Socket Created:  %d\n", s);
	fprintf(stdout, "Interface Index: %d\n", myifindex);

	return s;
}

static int seeifwouldblock(int sock) {
	fd_set fds;
	struct timeval timeout;
	int res;

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	FD_ZERO(&fds);
	FD_SET(sock, &fds);
	
//	fprintf(stderr, "select(): begin\n");

	res = select(FD_SETSIZE, &fds, NULL, NULL, &timeout);

//	fprintf(stderr, "select(): %d errno: %d\n", res, errno);

	if (FD_ISSET(sock, &fds)) {
		return 0;
	}
	return 1;
}

/* Certain events are handled by the C code... it is just more efficient */
int rawframe_recvFrame(int sock, char * dest, int len) {
	int keepGoing;
	int length;
	struct sockaddr_ll llsock;
	socklen_t llsocklen;
	
	keepGoing = 1;
	while (keepGoing) {
		llsocklen = sizeof(struct sockaddr_ll);
		/* if it would block return! */
		if (seeifwouldblock(sock)) {
			length = -1;
			break;
		}
		length = recvfrom(sock, dest, len, 0, (struct sockaddr *)&llsock, &llsocklen);
		if (dest[12] == 0x08 && dest[13] == 0x06) {
			/* this is ARP... reply, add to arp cache table or ignore */
			//arp_process_frame(sock, dest + 14, length - 14);
		}
		
		if (dest[12] == 0x08 && dest[13] == 0x00) {
			//fprintf(stdout, "%02x:%02x:%02x:%02x:%02x:%02x\n", dest[0] & 0xff, dest[1] & 0xff, dest[2] & 0xff, dest[3] & 0xff, dest[4] & 0xff, dest[5] & 0xff);
			//fprintf (stdout, "GOT IPv4 Packet!\n");
		}

		if ((dest[12] & 0xff) == 0x86 && (dest[13] & 0xff) == 0xdd) {
			fprintf(stdout, "0x%04x len %d\n", ((dest[12] << 8) & 0xff00) | (dest[13] & 0xff), length);
			//fprintf (stdout, "GOT IPv6 Packet!\n");
			keepGoing = 0;
		}
		//keepGoing = 0;
	}
	return length;
}

int quickdumpbuf(char * buf, int len) {
        int i;
        for (i = 0; i < len; i ++) {
                if (!(i % 16)) {
                        fprintf (stdout, "%04x ", i);
                }
                fprintf(stdout, "%02x ", buf[i] & 0xff);
                if (!((i + 1) % 16)) {
                        fprintf (stdout, "\n");
                }
        }
        fprintf(stdout, "\n");
}

void rawframe_sendFrame(int fd, char * buffer, int len) {
	struct sockaddr_ll socket_address;
	//quickdumpbuf(buffer, len);

	memset(&socket_address, 0, sizeof(socket_address));
	socket_address.sll_family   = PF_PACKET;
	socket_address.sll_ifindex  = myifindex;
	socket_address.sll_protocol = htons(ETH_P_ALL);
	socket_address.sll_pkttype  = PACKET_OTHERHOST;
	socket_address.sll_halen    = ETH_ALEN;

	//memcpy(buffer, gwmac, 6);
	memcpy(&buffer[6], mymac, 6);

	fprintf(stdout, "Sending Frame...\n");

	/* copy the mac into the sockaddr_ll structure */
	//memcpy(socket_address.sll_addr, buffer, 6);

	//quickdumpbuf(buffer, len);
	sendto(fd, buffer, len, 0, (struct sockaddr *)&socket_address, sizeof(socket_address));
}
