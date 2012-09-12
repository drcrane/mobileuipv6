#include <stdio.h>
#include <sys/select.h>
#include <string.h>
#include "packetsocketinterface.h"
#include "clock.h"

/* uIPv6 Headers */
#include "uip.h"
#include "uip-netif.h"
#include "uip-nd6.h"
#include "uip_arp.h"
#include "uip-netif.h"

/* Handle to the Packet Socket */
static int packetsocket;

#define ETH_BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define IP_BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

#define UIP_ICMP_BUF ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

static uint8_t eth_output(uip_lladdr_t *lladdr);
static void eth_poll();
void add_ipv6_address_bg();

static struct etimer periodical;
static int periodicalcounter;

static void oncepersecond(int ev, void * data);

int main(int argc, char *argv[]) {
	uint32_t tickclocka, tickclockb;
	struct timeval t;
	char * framebuffer;
	int pktsocket;

	clock_init();

	pktsocket = rawframe_getRawSocket("eth0");
	packetsocket = pktsocket;

	tickclocka = clock_time();

	fprintf(stdout, "Monotonic Time: %d\n", tickclocka);

	t.tv_sec = 1;
	t.tv_usec = 0;

	select(0, NULL, NULL, NULL, &t);

	tickclockb = clock_time();

	fprintf(stdout, "Monotonic Time: %d\n", tickclockb);
	fprintf(stdout, "Monotonic Time Difference: %d\n", tickclockb - tickclocka);

	framebuffer = malloc(4096);

/*
	while (clock_seconds() < 30) {
		fprintf(stdout, "%d ", clock_seconds());
		rawframe_recvFrame(pktsocket, framebuffer, 4096);
		fprintf(stdout, "\n");
	}
*/

	tcpip_set_outputfunc(eth_output);
	tcpip_init();
	clock_init();

	add_ipv6_address_bg();

	uip_listen(HTONS(23));

	etimer_set(&periodical, oncepersecond, CLOCK_SECOND);
	periodicalcounter = 0;

	for (;;) {
		eth_poll();
		etimer_poll();
		usleep(4000);
	}

	return 0;
}

static uint8_t eth_output(uip_lladdr_t *lladdr) {
    /* Setup MAC address */
    if (lladdr == NULL) {
        (&ETH_BUF->dest)->addr[0] = 0x33;
        (&ETH_BUF->dest)->addr[1] = 0x33;
        (&ETH_BUF->dest)->addr[2] = IP_BUF->destipaddr.u8[12];
        (&ETH_BUF->dest)->addr[3] = IP_BUF->destipaddr.u8[13];
        (&ETH_BUF->dest)->addr[4] = IP_BUF->destipaddr.u8[14];
        (&ETH_BUF->dest)->addr[5] = IP_BUF->destipaddr.u8[15];
    }
    else {
        memcpy(&ETH_BUF->dest, lladdr, UIP_LLADDR_LEN);
    }
    memcpy(&ETH_BUF->src, &uip_lladdr, UIP_LLADDR_LEN);
    ETH_BUF->type = HTONS(UIP_ETHTYPE_IPV6);
    uip_len += sizeof(struct uip_eth_hdr);

	/* Pass the frame to the ethernet driver */
	rawframe_sendFrame(packetsocket, (char *)uip_buf, uip_len);

	fprintf(stdout, "Sending Packet Complete!\n");
	return 0;
}

void uip_nd6_io_ns_input(void);

static void eth_poll() {
	//uip_len = ((char *)uip_buf, UIP_BUFSIZE);
	uip_len = rawframe_recvFrame(packetsocket, ETH_BUF, 4096);
	if (uip_len > 0 && uip_len != 0xffff) {
		if (ETH_BUF->type == htons(UIP_ETHTYPE_IPV6)) {
			fprintf (stdout, "\n****\n\n");
			/* OK, ipv6 frame! is this an ICMP Request? */
			if (IP_BUF->proto == 0x3a) {
				if ((UIP_ICMP_BUF->type & 0xff) == 135) {
					fprintf(stdout, "icmptype=%d\n", UIP_ICMP_BUF->type);
					//uip_nd6_io_ns_input();
					//eth_output(NULL);
				}
				if ((UIP_ICMP_BUF->type & 0xff) == 128) {
					//uip_icmp6_echo_request_input();
					//eth_output(NULL);
				}
			}
			//fprintf(stdout, "nexthdr=%d\n", IP_BUF->proto);
			//fprintf(stdout, "tcpip_input()\n");
			tcpip_input();
			//uip_input();
			//uip_process(UIP_DATA);
		}
	}
}

void ipv6_ethershield_process_data() {

//fprintf(stdout, "ipv6_ethershield_process_data!\n");
if (uip_newdata()) {
fprintf (stdout, "Connection: 0x%04x - %s %d\n", uip_conn, uip_newdata() ? "Y" : "N", uip_datalen());
((char *)uip_appdata)[uip_datalen()] = '\0';
fprintf (stdout, "[%s]\n", (char *)uip_appdata);
}

}

void uip_log(char *msg) {
	fprintf(stdout, "%s", msg);
}

void add_ipv6_address_bg() {
    uip_ipaddr_t ipv6_address;
    uip_ip6addr(&ipv6_address, 0xfe80, 0x0, 0x0, 0x0, 0x214, 0xa5ff, 0xfeef, 0xdb65);
    uip_netif_addr_add(&ipv6_address, 64, 0, MANUAL);
    uip_ip6addr(&ipv6_address, 0x2a00, 0xcafe, 0x0, 0x0, 0x0, 0x0, 0x0, 0xdb);
    uip_netif_addr_add(&ipv6_address, 64, 0, MANUAL);
    uip_ip6addr(&ipv6_address, 0x2a00, 0xcafe, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0);
    uip_nd6_prefix_add(&ipv6_address, 64, 0);
}

static char ello[40];

static void oncepersecond(int ev, void * data) {
	if (periodicalcounter == 20) {
		uip_ipaddr_t ipv6_address;
		fprintf(stdout, "opening connection...\n");
		uip_ip6addr(&ipv6_address, 0x2a00, 0xcafe, 0x0, 0x0, 0x0, 0x0, 0x0, 0xdc);
		tcp_connect(&ipv6_address, HTONS(2048), &ello);
	}

	fprintf(stderr, "Once a Second! - %d\n", periodicalcounter++);
	etimer_set(&periodical, oncepersecond, CLOCK_SECOND);
}
