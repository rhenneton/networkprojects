#include "headers.h" 
#define SIZEMAX 100000000


int fd;
sem_t semaphore;
pthread_mutex_t mutex;		//Mutex protégeant le file descriptor fd correspondant au socket 
pthread_mutex_t elemnowmut;	//Mutex protégeant l'entier elemnow
struct addrinfo* res=0;


int firstelem = 0;
int sber = 0;
int splr = 0;
int delay = 1;
int err;
int sendable = 1; 
int elemnow = 0;
int done =0;
int nbrpack; 
int huhuu=0;
void*resender(void*param);
void *ackreceiver(void * param);
struct addrinfo req, *ans;

int main(int argc,char * argv[])
{
	

	char* hostname; 
	char* portname;
	char * chaine=(char *) malloc(SIZEMAX*sizeof(char));
	
	
	int fs;		//File descriptor si lecture depuis fichier
	int text = 0; 	//flag indiquant si la lecture doit se faire depuis stdin (0) ou depuis un fichier (1)
	
	
	
	//Initialisation du sémaphore et des mutex
	
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
	


///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
////////    Récupération des différents arguments (hostname  //////////////////////////////////////////////
////////    et port) et des options: si nécessaire           //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////	
	
	
	
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
			read(fs,(void *) chaine,SIZEMAX*sizeof(char));

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
	hostname = argv[argc-2];
	portname = argv[argc-1];
	if(text==0)
	{
		fgets(chaine,SIZEMAX,stdin);
	}


	
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////   Création des paquets et du "gros buffer"        ///////////////////////////////
/////////////////////////   contenant l'ensemble des packets à envoyer      ///////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////	
	
//Note : Le nombre de bytes "reels" maximum dans un payload est 511 et pas 512, en effet, le dernier
//élément DOIT être un '\0' indiquant la fin de la chaine de caractères	
	
	int size = strnlen(chaine,SIZEMAX);
	int i;
	nbrpack = (size/511)+1;
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
		pack->seqnum = i%256;
		pack->length = strnlen(pack->payload,520);
		pack->crc = crc32(0,(void *) (pack),sizeof(packet)-4);
		grosbuf[i] = *pack;
		free(pack);
		pack=NULL;
		if(i==nbrpack-1)
		{
			huhuu=1;
		}
	}
	free(chaine);
	chaine = NULL;
	

///////////////////////////////////////////////////////////////////////////////////////////////////////////	
///////////////////////////////////////////////////////////////////////////////////////////////////////////	
//////////////////////////////  Fin de la partie récupération d'arguments et     //////////////////////////	
//////////////////////////////  préparation des différentes adresses             //////////////////////////	
///////////////////////////////////////////////////////////////////////////////////////////////////////////	
///////////////////////////////////////////////////////////////////////////////////////////////////////////	
	
	
	
	
	req.ai_flags = AI_PASSIVE;
 	req.ai_family = PF_INET6;    	     	//Nous utilisons IPv6
   	req.ai_socktype = SOCK_DGRAM;		//et des datagrammes			
   	req.ai_protocol = 0;
   	
   	
   	//Récupération des informations sur le host, port via getaddrinfo et création du socket
   	
   	
	getaddrinfo("::1",portname,&req,&ans);
	if ((fd = socket(ans->ai_family, ans->ai_socktype, ans->ai_protocol)) < 0) {
		perror("Impossible de créer le socket\n");
		return 0;
	}
	
	
	/*
	Lancement du ack-listener et du resender sur 2 threads
	*/
	
	pthread_t resend;
	pthread_t ackrec;
	
	err=pthread_create(&resend,NULL,&resender,&grosbuf);
	if(err!=0)
	{
		perror("ptread_create\n");
	}
	
	err=pthread_create(&ackrec,NULL,&ackreceiver,NULL);
	if(err!=0)
	{
		perror("ptread_create\n");
	}
	
	
	
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
		
		//Non envoi du packet
		if(random()%1000<splr)
		{
		
		}
		//Envoi du packet
		else
		{
			
			pthread_mutex_lock(&mutex);
			
			if (sendto(fd,&grosbuf[i],sizeof(packet),0,ans->ai_addr,ans->ai_addrlen)==-1) {
	
			}
			else
			{
				
			}
			pthread_mutex_unlock(&mutex);
		}
		//Rétablissement du packet (dans le buffer) à son état initial (sans corruption)
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
	free(ans);	
	close(fd);
	if(text==1)
	{
	close(fs);
	}
	err=pthread_join(resend,NULL); 
	if(err!=0) 
	perror("pthread_join");
	err=pthread_join(ackrec,NULL); 
	if(err!=0) 
	perror("pthread_join"); 
	return 0;
}




/*
Cette fonction est une sorte d'ack-listener, mettant à jour le numéro de l'élément actuel à recevoir
*/
void *ackreceiver(void * param)
{
		
	struct sockaddr_in remaddr;	
	socklen_t addrlen = sizeof(remaddr);		
	int recvlen;						
	int msgcnt = 0;			
	packet *buf = (packet*) malloc(sizeof(packet));	
	if(buf==NULL)
	{
		perror("erreur de malloc");
	}
	char* portnumber = "12345";
	int difference =0;
	int count = 0;
	int i;
	int lastacked = 0;
	while(done==0) 
	{
		recvlen = recvfrom(fd, buf, sizeof(packet), 0, (struct sockaddr *)&remaddr, &addrlen);
		if (recvlen > 0&&buf->type==2) {
			
			if(sendable < ((int)(buf->window))+1)
			{
				sendable = ((int)(buf->window))+1;
				for(i=0;i<buf->window;i++)
				{
					
					sem_post(&semaphore);
				}
			}
			
			pthread_mutex_lock(&elemnowmut);
			
			difference = buf->seqnum - elemnow;
			elemnow = ((buf->seqnum))% 256;
			if((int) buf->seqnum==nbrpack%256&&huhuu ==1)
			{
				
				if(buf->seqnum+count*256==nbrpack)
				{
					done=1;
				}
				
				
			}
			if((int) buf->seqnum==nbrpack%256)
			{
				count++;
			}
			pthread_mutex_unlock(&elemnowmut);
			
			for(i=0;i<difference;i++)
			{
				
				sem_post(&semaphore);
			}
			
			
			
		}
		
	}
	free(buf);
	buf = NULL;
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
		usleep(1500*delay);
		pthread_mutex_lock(&elemnowmut);
		nowfirstelem = elemnow;
		pthread_mutex_unlock(&elemnowmut);
		if(nowfirstelem==lastfirstelem)
		{
			
			
			
			i=lastfirstelem;
			while(i<lastfirstelem+sendable&&done==0&&i<nbrpack)
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
					
					
					sendto(fd,((currentpack)+i),sizeof(packet),0, ans->ai_addr,ans->ai_addrlen);
					
					
					
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
