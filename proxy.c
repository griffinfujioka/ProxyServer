#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include "DieWithMessage.c"

static int DEBUG = 1; 

static int MAX_MESSAGE_SIZE = 1024;

static int MAXPENDING = 5; 

#define PORT 2500 


int main()
{
	char messageBuffer[MAX_MESSAGE_SIZE]; 
	ssize_t numBytes = 0; 
	in_port_t receivingPort; 			// port for receiving messages 
	int sock; 						// socket for incoming connections 

	printf("================================================="); 
	printf("\nWelcome to the CptS 455 Proxy Server!"); 
	printf("\n\tCreated by: Griffin Fujioka"); 
	printf("\n\tNovember, 2013"); 
	printf("\n================================================="); 

	// TODO: Correct way to initialize our socket? 
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    	DieWithSystemMessage("socket() failed"); 

    /****************************************/ 
    /* Construct a local address structure 	*/ 
    /****************************************/ 
    struct sockaddr_in addr; 
    memset(&addr, 0, sizeof(addr)); 
    addr.sin_family = AF_INET;                                        // IPV4 address family 
    addr.sin_addr.s_addr = htonl(INADDR_ANY);         // any incoming interface 
    addr.sin_port = PORT; 

    /************************************/ 
    /* Bind to the local address 		*/ 
    /************************************/ 
	if(bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0)
		DieWithSystemMessage("bind() failed"); 

	/****************************************************************/ 
	/* Mark the socket so it will list for incoming connections 	*/ 
	/****************************************************************/
	if(listen(sock, MAXPENDING) < 0)
		DieWithSystemMessage("listen() failed");


	do
	{
		if(DEBUG)
			printf("\nServer ready for incoming connections..."); 

		struct sockaddr_in clntAddr; 		// client address

		// Set length of client address structure
		socklen_t clntAddrLen = sizeof(clntAddr);

		/************************************************************/ 
		/* Accept connections from web clients on port 2500 		*/ 
		/************************************************************/ 
		int clntSock = accept(sock, (struct sockaddr*) &clntAddr, &clntAddrLen); 

		if(clntSock < 0)
			DieWithSystemMessage("accept() failed"); 
		
		break; 
	}
	while(1); 



	printf("\nPress any key to exit..."); 
	char ch = getchar(); 
	return 0; 
}