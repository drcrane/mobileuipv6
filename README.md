Userspace uIPv6 for Linux (Intended for ARM)
============================================

This is a mini-IPv6 stack for use on Linux machines.

It makes use of the raw packet interface to send and receive ethernet 
frames on WLAN or on a wired ethernet network. The aim of this code is 
not for it to run on Linux but to run on bare ARM Cortex-M3 
hardware... and hopefully ARM Cortex-M3.

It is currently a base implementation taken from 
[Contiki](http://www.contiki-os.org) with help from 
(shapeshifter)[http://www.shapeshifter.se/code/uipv6/]. The current aim 
is to implement RFC 3775 (now obsolete, replaced by 6275) requirements, 
RFC 2473 (Generic Packet Tunneling in IPv6) and MIPv6 route optimisation 
mode. The code must also be updated with the code currently in Contiki.

# Compile the code

In the usual way: `make` from the `src/` directory.

# More information

Once running you should check you can ping the link local address:

ping6 -I eth0 fe80::217:8ff:fe36:e16f

Naturally replace the fe80::217:8ff:fe36:e16f with whatever link local 
IPv6 address is configured in your uIPv6 stack.

uip6.c and packetsocketinterface.c contain the MAC address of the 
interface ( you probably dont want this to match the 
actual mac address of your interface. )

# Disclamer

This code does work but it is certainly not production ready, I welcome 
any help!
