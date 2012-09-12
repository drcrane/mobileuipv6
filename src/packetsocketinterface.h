#ifndef __packetsocketinterface_h__
#define __packetsocketinterface_h__

int rawframe_getRawSocket(char * ifname);
int rawframe_recvFrame(int sock, char * dest, int len);
void rawframe_sendFrame(int fd, char * buffer, int len);

#endif
