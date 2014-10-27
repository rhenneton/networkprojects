
#include "headers.h"
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h> 

int fd;
sem_t semaphore;
pthread_mutex_t mutex;
pthread_mutex_t elemnowmut;
struct addrinfo* res=0;
int firstelem = 0;
int sber = 0;
int splr = 0;
int delay = 1;
int err;
int sendable = 32; 
int elemnow = 0;
int done =0;
int nbrpack; 

void*resender(void*param);
void *ackreceiver(void * param);


int main(int argc,char * argv[])
{
	

	const char* hostname; /* localhost */
	const char* portname;
	char * chaine=(char *) malloc(100000);
	int fs;
	int text = 0;
	
	err=sem_init(&semaphore, 0,sendable); 
	if(err!=0) 
	{ 
		error(err,"sem_init"); 
	} 
	err=pthread_mutex_init(&elemnowmut,NULL);
	if(err!=0)
	{
		error(err,"mutex_init");
	}
	err=pthread_mutex_init(&mutex,NULL);
	if(err!=0)
	{
		error(err,"mutex_init");
	}	
	
	/* Création du buffer A partir d'une séquence de bytes */
	
	
	
	int opt = 0;
	int long_index = 0;
	static struct option long_options[] = {
	{"file", required_argument,0,'a'},
	{"sber", required_argument,0,'b'},
	{"splr", required_argument,0,'c'},
	{"delay", required_argument,0,'d'}
	};
	char *end;
	while((opt = getopt_long(argc,argv,"a:b:c:d",long_options,&long_index)) != -1)
	{
		switch(opt)
		{
			case 'a':
			fs = open(optarg,O_RDONLY);
			read(fs,(void *) chaine,100000);

			text=1;
			
			break;
			case 'b':
			sber = (int)strtol(optarg,&end,10);
			
		
			break;
			case 'c':
			splr = (int) strtol(optarg,&end,10);
			break;
			case 'd':
			delay = (int)strtol(optarg,&end,10);
			
			break;
			default :
			printf("mauvaise option\n");
			
		}
		
		
		
	}
	
	if(text==0)
	{
		fgets(chaine,100000,stdin);
	}
	
	
	printf("sber = %d\n",sber);
	printf("splr = %d\n",splr);
	printf("delay = %d\n",delay);	
	hostname = argv[argc-2];
	portname = argv[argc-1];
	
	
	
	
	
	
	
	int size = strnlen(chaine,1000000);
	int i;
	nbrpack = (size/512)+1;
	packet grosbuf[nbrpack];
	int count;
	for(i=0;i<nbrpack;i++)
	{
		packet * pack = (packet*) malloc(sizeof(packet));
		bzero(pack, 520);
		for(count=0;count<511;count++)
		{
			pack->payload[count] = *(chaine+i*511+count);
			
		}
		pack->payload[count+1]='\0';
		pack->type = 1;
		pack->window = 0;
		pack->seqnum = i% 65535;
		pack->length = strnlen(pack->payload,520);
		pack->crc = crc32(0,(void *) (pack),sizeof(packet)-4);
		grosbuf[i] = *pack;
		
	}
	
	
	
	
	
	struct addrinfo hints;
	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_protocol=0;
	hints.ai_flags=AI_ADDRCONFIG;
	err=getaddrinfo(hostname,portname,&hints,&res);

	fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	if (fd==-1) {
	    
	}

	pthread_t resend;
	pthread_t ackrec;
	err=pthread_create(&resend,NULL,&resender,&grosbuf);
	err=pthread_create(&resend,NULL,&ackreceiver,NULL);
	int altered = 0;
	char caraltered;
	for(i=0;i<nbrpack;i++)
	{
		sem_wait(&semaphore);
		usleep(1000*delay);
		
		//Corruption du packet
		if(random()%1000<sber)
		{
			caraltered = grosbuf[i].payload[0];
			altered = 1;
			grosbuf[i].payload[0]+= 2;
		}
		if(random()%1000<splr)
		{
		
		}
		else
		{
			
			pthread_mutex_lock(&mutex);
			if (sendto(fd,&grosbuf[i],sizeof(packet),0,    res->ai_addr,res->ai_addrlen)==-1) {
	
			}
			else
			{
			
			}
			pthread_mutex_unlock(&mutex);
			
		}
		if(altered == 1)
		{
			altered = 0;
			grosbuf[i].payload[0] = caraltered;
		}
			
		
	
	
	}
	while(done==0)
	{
	
	}
	sem_destroy(&semaphore);	
	return 0;
}


void *ackreceiver(void * param)
{
		
	struct sockaddr_in remaddr;	
	socklen_t addrlen = sizeof(remaddr);		
	int recvlen;						
	int msgcnt = 0;			
	packet *buf = (packet*) malloc(sizeof(packet));	
	char* portnumber = "12345";
	int difference =0;
	int count = 0;
	int i;
	while(done==0) 
	{
		recvlen = recvfrom(fd, buf, sizeof(packet), 0, (struct sockaddr *)&remaddr, &addrlen);
		if (recvlen > 0&&buf->type==2) {
			
			
			pthread_mutex_lock(&elemnowmut);
			
			difference = buf->seqnum - elemnow;
			elemnow = ((buf->seqnum)-1)% 65535;
			if((int) buf->seqnum==nbrpack)
			{
				
				if(buf->seqnum+count*65536==nbrpack)
				{
					done=1;
				}
				
				count++;
			}
			pthread_mutex_unlock(&elemnowmut);
			for(i=0;i<difference;i++)
			{
				sem_post(&semaphore);
			}
			
			
			
		}
		
	}
}

void *resender(void*param)
{
	packet *currentpack = (packet *) param;	
	int nowfirstelem = 0;
	int lastfirstelem = 0;
	int i =0;
	int altered =0;
	char caraltered;
	while(done==0)
	{
		pthread_mutex_lock(&elemnowmut);
		lastfirstelem = elemnow;
		pthread_mutex_unlock(&elemnowmut);
		usleep(1500*delay*sendable);
		pthread_mutex_lock(&elemnowmut);
		nowfirstelem = elemnow;
		pthread_mutex_unlock(&elemnowmut);
		if(nowfirstelem==lastfirstelem)
		{
			
			
			
			i=lastfirstelem;
			while(i<lastfirstelem+sendable&&done==0)
			{
				pthread_mutex_lock(&elemnowmut);
				nowfirstelem = elemnow;
				pthread_mutex_unlock(&elemnowmut);
				if(nowfirstelem>i)
				{
					i=nowfirstelem;
				}	
				usleep(1000*delay);
					
				//Corruption du packet
				if(random()%1000<sber)
				{
					caraltered = ((currentpack)+i)->payload[0];
					altered = 1;
					((currentpack)+i)->payload[0]+= 2;
				}
				
				if(random()%1000<splr)
				{
		
				}
				else
				{
			
					pthread_mutex_lock(&mutex);
					sendto(fd,((currentpack)+i),sizeof(packet),0, res->ai_addr,res->ai_addrlen);
						
					
					
					pthread_mutex_unlock(&mutex);
			
				}
				if(altered ==1)
				{
					altered =0;
					((currentpack)+i)->payload[0]= caraltered;
				}
				i++;
			
			}		
		}	
	
	}
}
