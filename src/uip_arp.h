#ifndef __uip_arp_h__
#define __uip_arp_h__

#include "uip.h"

CCIF extern struct uip_eth_addr uip_ethaddr;

/**
 * The Ethernet header.
 */
struct uip_eth_hdr {
  struct uip_eth_addr dest;
  struct uip_eth_addr src;
  u16_t type;
};

#define UIP_ETHTYPE_IPV6 0x86dd

#endif // __uip_arp_h__
