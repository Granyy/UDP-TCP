#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>


/*************************************************************************************************/
/*					CONSTRUIRE_MESSAGE   					 */
/*************************************************************************************************/

		void construire_message(char* message, char motif, int lg)
		{

		int i ;

		for (i=0;i<lg;i++) message[i]=motif ;
		}




/*************************************************************************************************/
/*					AFFICHE_MESSAGE   					 */
/*************************************************************************************************/

		void afficher_message(char* message, int lg)
		{
		int i ;

		for (i=0;i<lg;i++) printf("%c",message[i]) ;
		}





/*************************************************************************************************/
/*						SOURCE_UDP					 */
/*************************************************************************************************/


		void source_process_UDP(int port, char* nom_station, char* pmessage, int n, int longueur_message)
		{
		
		int socket_source ;
		struct sockaddr_in adr_local ;
		struct hostent *hp ;
		struct sockaddr_in adr_distant ;
		int lg_mesg=longueur_message ;
		int lg_adr_dest ;

		int i ;
		
		/* création d'un socket LOCAL*/
		if ((socket_source=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))==-1)
		{
			printf("Echec de creation du socket \n") ;
			exit(1) ;
		}
		
		/* construction de l'@ du socket avec n° de port en argument (LOCAL)*/
		memset((char*)&adr_local,0,sizeof(adr_local)) ; //RESET
		adr_local.sin_family=AF_INET ;
		adr_local.sin_port=port ;
		adr_local.sin_addr.s_addr=INADDR_ANY ;

		/* construction de l'@ du socket avec n° de port en argument (DISTANT)*/		
		memset((char*)&adr_distant,0,sizeof(adr_distant)) ;
		adr_distant.sin_family=AF_INET ;
		adr_distant.sin_port=port ;
		
		/*Obtention de l'adresse socket distante*/		
		if ((hp=gethostbyname(nom_station))==NULL)
		{
			printf("Echec Gethostbyname\n") ;
			exit(1) ;
		}
		memcpy((char*)&(adr_distant.sin_addr.s_addr),hp->h_addr,hp->h_length) ;
		lg_adr_dest=sizeof(adr_distant) ;


		/*Envois des messages*/
		printf("SOURCE : lg_msg_lu=%d, port=%d, nb_envois=%d,TP=UDP \n",lg_mesg,port,n) ;

		for (i=0;i<n;i++)
		{	
			/*Construction d'un message : N° (sur 5 digits) + lettre répétée sur le reste du message */
			sprintf(pmessage,"%05d",(i+1)) ;
			construire_message(pmessage+5,('a'+(i%26)),longueur_message-5) ;

			/*Envoi du message créé*/
			sendto(socket_source,pmessage,lg_mesg,0,(struct sockaddr*)&adr_distant,lg_adr_dest) ;
			printf("SOURCE: Envoi n°%d (%d) [",i,lg_mesg) ;
			afficher_message(pmessage, longueur_message) ;
			printf("]\n") ;	
		}

		close(socket_source) ;
		}
		



/*************************************************************************************************/
/*						SOURCE_TCP					 */
/*************************************************************************************************/


		void source_process_TCP(int port, char* nom_station, char* pmessage, int nb_message, int longueur_message)
		{
		int socket_source ;
		struct sockaddr_in adr_local ;
		struct hostent *hp ;
		struct sockaddr_in adr_distant ;


		int lg_mesg=longueur_message ;
		int i ;

		/* création d'un socket LOCAL*/
		if ((socket_source=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==-1)
		{
		printf("Echec de creation du socket \n") ;
		exit(1) ;
		}
		
		/* construction de l'@ du socket avec n° de port en argument LOCAL*/
		memset((char*)&adr_local,0,sizeof(adr_local)) ; //ça fait un reset, au cas où
		adr_local.sin_family=AF_INET ;
		adr_local.sin_port=port ;
		adr_local.sin_addr.s_addr=INADDR_ANY ;

		/* construction de l'@ du socket avec n° de port en argument DISTANT*/		
		memset((char*)&adr_distant,0,sizeof(adr_distant)) ;
		adr_distant.sin_family=AF_INET ;
		adr_distant.sin_port=port ;
		

		/*Obtention de l'adresse socket distante*/
		if ((hp=gethostbyname(nom_station))==NULL)
		{
			printf("Echec Gethostbyname\n") ;
			exit(1) ;
		}
		memcpy((char*)&(adr_distant.sin_addr.s_addr),hp->h_addr,hp->h_length) ; 
		
		
		/* Etablissement d'une demande de connexion*/
		if ((connect(socket_source,(struct sockaddr*)&adr_distant,sizeof(adr_distant)))==-1)
		{
			printf("Echec du connect\n") ;
			exit(1) ;
		}
		
		/*Envois des messages*/
		printf("SOURCE : lg_msg_lu=%d, port=%d, nb_envois=%d,TP=TCP \n",lg_mesg,port,nb_message) ;

		for (i=0;i<nb_message;i++)
		{	
			/*Construction d'un message : N° (sur 5 digits) + lettre répétée sur le reste du message */
			sprintf(pmessage,"%05d",(i+1)) ;
			construire_message(pmessage+5,('a'+(i%26)),longueur_message-5) ;

			printf("SOURCE: Envoi n°%d (%d) [",(i+1),lg_mesg) ;

			/*Envoi du message créé*/
			if ((write(socket_source,pmessage,lg_mesg))==-1)
			{
				printf("Echec du write\n") ;
				exit(1) ;
			}

			afficher_message(pmessage, longueur_message) ;
			printf("]\n") ;	
		}
		
		shutdown(socket_source,2) ;
		close(socket_source) ;
	
		}




/*************************************************************************************************/
/*						PUITS_UDP   					 */
/*************************************************************************************************/


		void puits_process_UDP(int port, char* pmessage, int nb_message, int longueur_message)
		{

		int socket_recepteur ;
		struct sockaddr_in adr_local ;
		int lg_adr_local=sizeof(adr_local) ;
		int lg_max=longueur_message ;
		int* plg_adr_em ;
		struct sockaddr *padr_em ;

		int i=0 ;

		
		
		/* création d'un socket LOCAL*/
		if ((socket_recepteur=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))==-1)
		{
			printf("Echec de creation du socket \n") ;
			exit(1) ;
		}

		/* construction de l'@ du socket avec n° de port en argument*/
		memset((char*)&adr_local,0,sizeof(adr_local)) ; //ça fait un reset, au cas où
		adr_local.sin_family=AF_INET ;
		adr_local.sin_port=port ;
		adr_local.sin_addr.s_addr=INADDR_ANY ;

		/*association de l'@ socket avec sa représentation interne */
		if (bind(socket_recepteur,(struct sockaddr *)&adr_local, lg_adr_local)==-1)
		{
			printf("Echec du bind\n") ;
			exit(1) ;
		}
		
		/*Réception des messages*/
		printf("PUITS : lg_msg_lu=%d, port=%d, nb_receptions=infini,TP=UDP \n",lg_max,port) ;


		while (1)
		{	i++ ;
			recvfrom(socket_recepteur,pmessage,lg_max,0,padr_em,plg_adr_em) ;
			printf("PUITS : Reception n°%d (%d) [",i,lg_max) ;
			afficher_message(pmessage, longueur_message) ;
			printf("]\n") ;	
		}	

	
		close(socket_recepteur) ;		

		}




/*************************************************************************************************/
/*						PUITS_TCP  					 */
/*************************************************************************************************/
		
		void puits_process_TCP(int port, char* pmessage, int nb_message, int longueur_message)
		{
		int socket_recepteur ;
		struct sockaddr_in adr_local ;
		int lg_adr_local=sizeof(adr_local) ;
		int lg_max=longueur_message; 
		struct sockaddr_in adr_client ;
		int lg_adr_client=sizeof(adr_client) ;
		int lg_rec ;
		int socket_connexion;
		
		int i=0 ;

		/* création d'un socket LOCAL*/
		if ((socket_recepteur=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==-1)
		{
			printf("Echec de creation du socket \n") ;
			exit(1) ;
		}

		/* construction de l'@ du socket avec n° de port en argument*/
		memset((char*)&adr_local,0,sizeof(adr_local)) ; //ça fait un reset, au cas où
		adr_local.sin_family=AF_INET ;
		adr_local.sin_port=port ;
		adr_local.sin_addr.s_addr=INADDR_ANY ;

		/*association de l'@ socket avec sa représentation interne */

		if (bind(socket_recepteur,(struct sockaddr *)&adr_local, lg_adr_local)==-1)
		{
			printf("Echec du bind\n") ;
			exit(1) ;
		}

			
		if (listen(socket_recepteur,30)==-1)
		{
			printf("Echec du listen\n") ;
			exit(1) ;
		}

		if ((socket_connexion=accept(socket_recepteur,(struct sockaddr *)&adr_client,&lg_adr_client))==-1)
		{
			printf("Echec du accept\n") ;
			exit(1) ;
		}
	
		/*Réception des messages*/
		printf("PUITS : lg_msg_lu=%d, port=%d, nb_receptions=%d,TP=TCP \n",lg_max,port,nb_message) ;
		
		
		while (((lg_rec=read(socket_connexion,pmessage,lg_max))!=0)&&(lg_rec!=1)&&((nb_message>i)||(nb_message==-1)))   //soit le read echoue, 																	//soit l'emetteur ferme la connexion, 		
		{														//soit on est limité par le nombre de message		
			i++;													
			printf("PUITS : Reception n°%d (%d) [",i,lg_max) ;
			afficher_message(pmessage, longueur_message) ;
			printf("]\n") ;	
		}
		

		shutdown(socket_recepteur,2) ;		
		shutdown(socket_connexion,2) ;
		close(socket_recepteur) ;
		close(socket_connexion) ;		
		
		}				





/*************************************************************************************************/
/*						MAIN   						 */
/*************************************************************************************************/


main (int argc, char **argv)
{
	int c;
	int port;
	char* nom_station ;
	int protocole=0 ; //TCP 
	extern char *optarg;
	extern int optind;
	int longueur_message=-1 ;
	char* pmessage ;
	int nb_message = -1; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
	int source = -1 ; /* 0=puits, 1=source */
	while ((c = getopt(argc, argv, "psn:ul:")) != -1) {
		switch (c) {
		case 'p':
			if (source == 1) {
				printf("usage: cmd [-p|-s][-n ##]\n");
				exit(1);
			}
			source = 0;
			break;

		case 's':
			if (source == 0) {
				printf("usage: cmd [-p|-s][-n ##]\n");
				exit(1) ;
			}
			source = 1;
			break;
		case 'u':
			protocole=1 ; //UDP
			break ;
		case 'n':
			nb_message = atoi(optarg);
			break;
		case 'l':
			longueur_message = atoi(optarg);
			
			break;
		default:
			printf("usage: cmd [-p|-s][-n ##]\n");
			break;
		}
	}

	/*Allocation de memoire pour le message*/
	if (longueur_message == -1) longueur_message=30 ;
	pmessage=(char*)malloc(longueur_message);


	if (nb_message != -1) {
		if (source == 1)
			printf("nb de tampons à envoyer : %d\n", nb_message);
		else
			printf("nb de tampons à recevoir : %d\n", nb_message);
	} else {
		if (source == 1) {
			nb_message = 10 ;
			printf("nb de tampons à envoyer = 10 par défaut\n");
		} else
		printf("nb de tampons à envoyer = infini\n");
	}

	if (source == -1) {
		printf("usage: cmd [-p|-s][-n ##]\n");
		exit(1) ;
	}

	if (source == 1)
	{
		printf("on est dans la source\n");
		port=atoi(argv[argc-1]);
		port=htons(port);
		nom_station=argv[argc-2] ;
		if (protocole==1) source_process_UDP(port, nom_station,pmessage, nb_message, longueur_message);
		else source_process_TCP(port, nom_station,pmessage, nb_message, longueur_message);
		
	}

	else
	{
		printf("on est dans le puits\n");
		port=atoi(argv[argc-1]);
		port=htons(port);
		if (protocole==1) puits_process_UDP(port,pmessage, nb_message,longueur_message);
		else puits_process_TCP(port,pmessage, nb_message, longueur_message) ;
	}


	free(pmessage);
}

