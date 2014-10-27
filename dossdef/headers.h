#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>
#include <zlib.h>
#include <ctype.h>

#define BUFLEN 2048
#define MSGS 5	/* number of messages to send */

typedef struct packet
{
   uint8_t type : 3;
   uint8_t window : 5;
   uint8_t seqnum : 8;
   uint16_t length : 16;
   char payload[512];
   signed int crc;
} packet;

long strntol (const char *str, const char *end) {
	int i = 0;
	long res = 0;
	while (str+i<end) {
		if (!isdigit(*(str+i))) return 0;
		res = res*10 + *(str+i)-'0';
		++i;
		}
	return res;
	}
