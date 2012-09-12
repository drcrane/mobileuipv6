CC=gcc
CFLAGS=-g -c -Wall -DDEBUG
LDFLAGS=-lrt
SOURCES=clock.c clockmain.c etimer.c packetsocketinterface.c random.c tcpip.c timer.c uip-icmp6.c uip-nd6-io.c uip-nd6.c uip-netif.c uip-split.c uip6.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=ipv6server

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o
	rm $(EXECUTABLE)

