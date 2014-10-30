#include "headers.h"
#define WINDOWSSIZE 31
   
int lastreceived = 0;
packet buffer[32];
int subbuf[32];


int isinbuff(int seqnum)
{
	int i;
	int resp = 0;
	for(i=0;i<32;i++)
	{
		if(subbuf[i]>-1)
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
	
	
	
	int fs; 		//File descriptor 
	int text =0;		//flag indiquant si l'écriture doit se faire sur stdout (0) ou sur fichier (1)
	int i = 0;		//Eternelle variable dédiée aux boucles for


	
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
////////    Récupération des différents arguments (hostname et port) et ///////////////////////////////////
////////    des options:ouverture du fichier "filename" si nécessaire   ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	int opt = 0;		
	int long_index = 0;
	static struct option long_options[] = {
	{"file", required_argument,0,'a'}
	};
	
	while((opt = getopt_long(argc,argv,"a",long_options,&long_index)) != -1)
	{
		switch(opt)
		{
			case 'a':
			fs = open(optarg,O_WRONLY|O_CREAT,chmod(optarg,00666));
			text=1;
			
			break;
			default :
			printf("Mauvaise option\n");
			
		}
	
	}	
	char* portnumber = argv[argc-1];
	char* hostname = argv[argc-2];	
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////	
///////////////////////////////////////////////////////////////////////////////////////////////////////////	
//////////////////////////////  Fin de la partie récupération d'arguments et     //////////////////////////	
//////////////////////////////  préparation des différentes adresses et rcvfrom  //////////////////////////	
///////////////////////////////////////////////////////////////////////////////////////////////////////////	
///////////////////////////////////////////////////////////////////////////////////////////////////////////



	struct addrinfo req, *ans;		//Addresse de notre machine
	struct sockaddr_in6 remaddr;		//Adresse du sender, récupérée à la réception du premier packet
	socklen_t addrlen = sizeof(remaddr);		
	
	int recvlen;			
	int fd;				
	
	
	packet *buf = (packet*) malloc(sizeof(packet));	 //buffer de réception des packets
	req.ai_flags = AI_PASSIVE;
   	req.ai_family = PF_INET6;            	//Utilisation d'IPv6 
   	req.ai_socktype = SOCK_DGRAM;		//et de datagrammes
   	req.ai_protocol = 0;
   	
   	//Récupération des informations sur le host, port via getaddrinfo et création du socket
   				
	getaddrinfo(hostname,portnumber,&req,&ans);
	if ((fd = socket(ans->ai_family, ans->ai_socktype, ans->ai_protocol)) < 0) {
		perror("Impossible de créer le socket\n");
		return 0;
	}

	//assignation de l'adresse au socket
	
	if (bind(fd,ans->ai_addr,ans->ai_addrlen) < 0) {
		perror("Erreur de bind");
		return 0;
	}
	
	for(i=0;i<32;i++)
	{
		subbuf[i]=-1;
	}
	
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////	
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////   Réception des différents packets   ///////////////////////////////////////
//////////////////////////////   et traitement du selective repeat  ///////////////////////////////////////	
///////////////////////////////////////////////////////////////////////////////////////////////////////////	
///////////////////////////////////////////////////////////////////////////////////////////////////////////	
	
	int done = 0;		//Arret du receiver une fois qu'un paquet de taille 
	printf("Waiting on port %s\n", portnumber);
	packet * ack = (packet*) malloc(sizeof(packet));
	while(done==0) {

		recvlen = recvfrom(fd, buf, sizeof(packet), 0, (struct sockaddr *)&remaddr, &addrlen);
		
		
		//Vérification du crc
		if (recvlen > 0 && (int) buf->crc == (int) crc32(0,(void *) (buf),sizeof(packet)-4)) 
		{
			//Si le packet reçu est valide :
			
			
			
			
		
			ack->type = 2;
			ack->window = WINDOWSSIZE;
			
			if(((int) buf->seqnum)-lastreceived>=1)
			{
				
				//Un packet valide a été recu, malheureusement il n'est pas le paquet suivant
				//Il faut donc l'ajouter dans le buffer s'il n'y figure pas encore
				
				packet * packtoadd =(packet *) malloc(sizeof(packet));
				packtoadd->type = buf->type;
				packtoadd->window = buf->window;
				packtoadd->length = buf->length;
				packtoadd->crc = buf->crc;
				packtoadd->seqnum = buf->seqnum;
				
				for(i=0;i<buf->length;i++)
				{
					packtoadd->payload[i]=buf->payload[i];
				}
				
				
				//On ajoute l'élément dans le buffer (si il n'est pas encore dans le buffer)
				if(isinbuff((int) packtoadd->seqnum)==0)
				{
					for(i=0;i<32;i++)
					{
						if(subbuf[i]<0)
						{
							buffer[i] = *packtoadd;
							subbuf[i] = packtoadd->seqnum;
							
							i=33;
						}
					}
				}
				
				
				
			}
			else
			{
				
				//Un paquet valide est recu et est le dernier packet (length <511), on peut donc arreter le receiver
			
				if(buf->length<511)
				{
					done = 1;
				}
			
				
				if(lastreceived ==(int) buf->seqnum)
				{
					
					//Ecriture du paquet sur stdout ou dans le fichier spécifié dans les options
					
					if(text==1)
					{
						
						write(fs,(void *) buf->payload,strnlen(buf->payload,520));
					}
					else
					{
						printf("%s",buf->payload);
						printf("\n%d\n",buf->seqnum);	
					}
					lastreceived = ((buf->seqnum)+1)%256;
				}
				
				/*
				Une fois que le payload a été écrit avant d'envoyer l'ack, il faut vérifier que le paquet suivant
				ne se trouve pas dans le buffer (selective repeat), puis le suivant,...
				*/
				for(i=1;i<32;i++)
				{ 
					
					if(isinbuff(((int) buf->seqnum)+i)==1)
					{
						int j;
						for(j=0;j<32;j++)
						{
							if((int) buffer[j].seqnum==((int) (buf->seqnum))+i)
							{
								subbuf[i] = -1;
								if(buffer[j].length<511)
								{
									done = 1;
								}
								if(text==1)
								{
									
									write(fs,(void *) buffer[j].payload,strnlen(buf->payload,520));
								}
								else
								{
									printf("%s",buffer[j].payload);
								}
									lastreceived = ((buffer[j].seqnum)+1)%256;
									j=33;
							}
						}
					}
					else
					{
						i = 33;
					}
				
				
				}
				
				ack->seqnum = ((lastreceived))%256;
				if (sendto(fd, ack, sizeof(packet), 0, (struct sockaddr *)&remaddr, addrlen) < 0);
				
			}
			//Envoi de l'acknoledgement
			
			
			
		}
		

	}
	free(buf);
	buf = NULL;
	free(ack);
	ack =NULL;
	free(ans);
	ans =NULL;
	
	return 0;	
}
