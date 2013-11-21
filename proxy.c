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
	int sock; 							// socket for incoming connections 
	int childCount = 0; 				// number of children processes currently running 
	int pid = 0; 
	int iterationCounter = 0; 

	printf("================================================="); 
	printf("\nWelcome to the CptS 455 Proxy Server!"); 
	printf("\n\tCreated by: Griffin Fujioka"); 
	printf("\n\tNovember, 2013"); 
	printf("\n================================================="); 

	/************************************************************/ 
	/* Create a socket for handling new connection requests 	*/ 
	/************************************************************/ 
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    	DieWithSystemMessage("socket() failed"); 

    /****************************************/ 
    /* Construct a local address structure 	*/ 
    /****************************************/ 
    struct sockaddr_in addr; 
    memset(&addr, 0, sizeof(addr)); 
    addr.sin_family = AF_INET;                           	// IPV4 address family 
    addr.sin_addr.s_addr = htonl(INADDR_ANY);         		// any incoming interface 
    addr.sin_port = htons(PORT); 							// Port to be bound to 

    /************************************/ 
    /* Bind to the local address 		*/ 
    /************************************/ 
	if(bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0)
		DieWithSystemMessage("bind() failed"); 

	if(DEBUG)
		printf("\nSuccessfully bound socket #%d to port #%d", sock, PORT); 

	/****************************************************************/ 
	/* Mark the socket so it will list for incoming connections 	*/ 
	/****************************************************************/
	if(listen(sock, MAXPENDING) < 0)
		DieWithSystemMessage("listen() failed");

	if(DEBUG)
		printf("\nSuccessfully marked socket #%d for listening", sock); 


	do
	{
		iterationCounter++; 
		printf("\nIteration %d: \n\n", iterationCounter); 
		if(DEBUG)
			printf("\nServer ready for incoming connections..."); 

		struct sockaddr_in clntAddr; 		// client address

		socklen_t clntAddrLen = sizeof(clntAddr);		// Set length of client address structure

		/************************************************************/ 
		/* Accept connections from web clients on port 80 		*/ 
		/************************************************************/ 
		int clntSock = accept(sock, (struct sockaddr*) &clntAddr, &clntAddrLen); 

		if(clntSock < 0)
			DieWithSystemMessage("accept() failed"); 

		if(DEBUG)
			printf("\nSuccessfully accepted connection. Client is using socket %d", clntSock); 

		char clntName[INET_ADDRSTRLEN];                 // String for client address
        if(inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName, sizeof(clntName)) != 0)
        {
        	printf("\nHandling client on socket #%d: \n\tIP address: %s\n\tPort #: %d\n", clntSock, clntName, clntAddr.sin_port);

        	/* Receive some data from the client */ 
        	numBytes = recv(clntSock, messageBuffer, MAX_MESSAGE_SIZE, 0);

        	if(numBytes < 0)
            	DieWithSystemMessage("recv() failed\n"); 
            else if(numBytes == 0)
                    DieWithUserMessage("recv()", "connection closed prematurely\n"); 

            if(DEBUG)
           		printf("\nReceived %zu bytes from the client: %s", numBytes, messageBuffer); 
        }

		/****************************************************************/ 
		/* Fork a process to handle communication for this request 		*/ 
		/****************************************************************/ 
		// if(fork() == 0)
		// {
		// 	close(sock); 
		// 	do
		// 	{
		// 		// Do stuff as long as we're talking to the client 
		// 	}
		// 	while(1); 

		// 	close(clntSock); 
		// }

		// close(clntSock); 
		// childCount++; 

		// while(childCount)
		// {
		// 	pid = waitpid(-1, NULL, WNOHANG); 
		// 	if(pid == 0)
		// 		break; 
		// 	else
		// 		childCount--; 
		// }
		
		//break; 
	}
	while(1); 



	printf("\nPress any key to exit..."); 
	char ch = getchar(); 
	return 0; 
}