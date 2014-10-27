#include "headers.h"
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h> 

int lastreceived = 0;
packet buffer[32];
pthread_mutex_t buffermut;

int isinbuff(int seqnum)
{
	int i;
	int resp = 0;
	for(i=0;i<32;i++)
	{
		if(buffer[i].seqnum>-1)
		{
			if(seqnum==(int) buffer[i].seqnum)
			{
				i=33;
				resp = 1;
			}
		}
	}
	return resp;
}

int main(int argc, char **argv)
{
	
	pthread_mutex_init(&buffermut,NULL);
	
	int fs;
	int text =0;
	int opt = 0;
	int long_index = 0;
	static struct option long_options[] = {
	{"file", required_argument,0,'a'}
	};
	char end;
	while((opt = getopt_long(argc,argv,"a",long_options,&long_index)) != -1)
	{
		switch(opt)
		{
			case 'a':
			fs = open(optarg,O_WRONLY|O_CREAT,chmod(optarg,00777));
			text=1;
			
			break;
			default :
			printf("mauvaise option\n");
			
		}
		
		
		
	}	
	
	
	

	/*Réception d'un paquet et impression du type et payload :) */
	
	
	
	
	
	int i = 0;
	
	
	struct sockaddr_in myaddr;	/* our address */
	struct sockaddr_in remaddr;	/* remote address */
	socklen_t addrlen = sizeof(remaddr);		/* length of addresses */
	int recvlen;			/* # bytes received */
	int fd;				/* our socket */
	int msgcnt = 0;			/* count # of messages we received */
	char* portnumber = argv[argc-1];
	packet *buf = (packet*) malloc(sizeof(packet));	/* receive buffer */
	

	/* create a UDP socket */

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return 0;
	}

	/* bind the socket to any valid IP address and a specific port */

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(12345);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}

	/* now loop, receiving data and printing what we received */
	int done = 0;
	printf("waiting on port %s\n", portnumber);
	
	while(done==0) {

		recvlen = recvfrom(fd, buf, sizeof(packet), 0, (struct sockaddr *)&remaddr, &addrlen);
		
		if (recvlen > 0 && (int) buf->crc == (int) crc32(0,(void *) (buf),sizeof(packet)-4)) 
		{
			
			
			
			
			packet * ack = (packet*) malloc(sizeof(packet));
		
			ack->type = 2;
			ack->window = 31;
			if(((int) buf->seqnum)-lastreceived>=1)
			{
				//Un packet valide a été recu, malheureusement il n'est pas le paquet suivant :(
				packet * packtoadd = (packet*) malloc(sizeof(packet));
				packtoadd->type = buf->type;
				packtoadd->window = buf->window;
				packtoadd->length = buf->length;
				packtoadd->crc = buf->crc;
				packtoadd->seqnum = lastreceived+1;
				for(i=0;i<buf->length;i++)
				{
					packtoadd->payload[i]=buf->payload[i];
				}
				pthread_mutex_lock(&buffermut);
				
				//On ajoute l'élément dans le buffer :)
				if(isinbuff((int) packtoadd->seqnum)==0)
				{
					for(i=0;i<32;i++)
					{
						if(buffer[i].type!=1)
						{
							buffer[i] = *packtoadd;
							printf("Packet n°%d ajouté au buffer en position%d\n\n",(int) packtoadd->seqnum,i);
							i=33;
						}
					}
				}
				pthread_mutex_unlock(&buffermut);
				
				
			}
			else
			{
				
				
				if(buf->length<511 && lastreceived ==(int) buf->seqnum)
				{
					done = 1;
				}
				
				if(lastreceived ==(int) buf->seqnum)
				{
					
					if(text==1)
					{
						write(fs,(void *) buf->payload,strnlen(buf->payload,520));
					}
					else
					{
						printf("%s\n\n",buf->payload);
					}
					lastreceived++;
				}
				
				
				
				
				
				
				
				
				for(i=1;i<32;i++)
				{
					
					if(isinbuff(((int) buf->seqnum)+i)==1)
					{
						int j;
						for(j=0;j<32;j++)
						{
							if((int) buffer[j].seqnum==((int) (buf->seqnum))+i)
							{
								buffer[j].seqnum = -1;
							
								if(text==1)
								{
									write(fs,(void *) buffer[j].payload,strnlen(buf->payload,520));
								}
								else
								{
									printf("%s\n\n",buffer[j].payload);
								}
									lastreceived++;
									j=33;
							}
						}
					}
					else
					{
						i = 33;
					}
				
				
				}


				
				
				
				
				
				
				
				
				
				
				
				
				ack->seqnum = (buf->seqnum)+1;
				
				
			}
			if (sendto(fd, ack, sizeof(packet), 0, (struct sockaddr *)&remaddr, addrlen) < 0);
				
				
		
		
		
		
		
		
		
	
			
				
		}
		else
			printf("uh oh - something went wrong!\n");
		
		
		
	}
	return 0;	
}
