#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>
#include <zlib.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>


typedef struct packet
{
   uint8_t type : 3;
   uint8_t window : 5;
   uint8_t seqnum : 8;
   uint16_t length : 16;
   char payload[512];
   signed int crc;
} packet;

