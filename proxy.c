#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include "DieWithMessage.c"

static int DEBUG = 1; 

static int MAX_MESSAGE_SIZE = 1024;

static int MAXPENDING = 5; 

static int SIZEOF_HTTP_OPERATION = 5; 

static int SIZEOF_PATH_TO_FILE = 256; 

static int SIZEOF_HTTP_VERSION = 24; 

static int SIZEOF_HOSTNAME = 256; 

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

	char httpOperation[SIZEOF_HTTP_OPERATION]; 		// I.e., GET, POST, HEAD
	char pathToFile[SIZEOF_PATH_TO_FILE]; 			// I.e., http://www.cnn.com
	char httpVersion[SIZEOF_HTTP_VERSION]; 			// I.e., HTTP/1.1
	char host[SIZEOF_HOSTNAME]; 					// I.e., www.cnn.com 

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

		/****************************************/
		/* Zero out all important buffers 		*/
		/****************************************/  
		memset(&messageBuffer, 0, MAX_MESSAGE_SIZE); 			// zero the message buffer 
		memset(&httpOperation, 0, SIZEOF_HTTP_OPERATION); 		// zero the HTTP operation buffer
		memset(&pathToFile, 0, SIZEOF_PATH_TO_FILE); 			// zero the path to file buffer 
		memset(&httpVersion, 0, SIZEOF_HTTP_VERSION); 			// zero the HTTP version buffer
		memset(&host, 0, SIZEOF_HOSTNAME); 						// zero the host buffer 

		struct sockaddr_in clntAddr; 		// client address

		socklen_t clntAddrLen = sizeof(clntAddr);	

		/************************************************************/ 
		/* Accept connections from web clients on PORT				*/ 
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

        	/*************************************/ 
        	/* Receive some data from the client */ 
        	/*************************************/ 
        	numBytes = recv(clntSock, messageBuffer, MAX_MESSAGE_SIZE, 0);

        	if(numBytes < 0)
            	DieWithSystemMessage("recv() failed\n"); 
            else if(numBytes == 0)
            {
            	if(DEBUG)
            		printf("\nrecv() failed: No data received."); 

            	continue;		// go back to the beginning of the infinite loop  
            }

            int tokenizer = 0; 			// used to tokenize the HTTP request headers into OP /path/to/file HTTP/1.x 
            int i = 0; 					// used for filling up respective buffers with information from the HTTP request headers

            /********************************************************/ 
        	/* Parse messageBuffer to attain the HTTP operation 	*/ 
            /********************************************************/  
            while(messageBuffer[tokenizer] != ' ')
            {
            	httpOperation[i] = messageBuffer[tokenizer]; 
            	tokenizer++; 
            	i++; 
            }

            httpOperation[i] = '\0'; 
            tokenizer++; 
            i=0; 

            /********************************************************/ 
            /* Parse messageBuffer to attain the path to the file 	*/ 
            /********************************************************/ 
            while(messageBuffer[tokenizer] != ' ')
            { 
            	pathToFile[i] = messageBuffer[tokenizer]; 
            	tokenizer++; 
            	i++; 
            }

            pathToFile[i] = '\0'; 
            tokenizer++; 
            i=0; 

            /********************************************************/ 
            /* Parse messageBuffer to attain the HTTP version 		*/ 
            /********************************************************/ 
            while(messageBuffer[tokenizer] != '\r')
            { 
            	httpVersion[i] = messageBuffer[tokenizer]; 
            	tokenizer++; 
            	i++; 
            }

            httpVersion[i] = '\0'; 
            tokenizer++; 
            i=0; 

            /********************************************************/ 
            /* Parse messageBuffer to attain host name 				*/ 
            /********************************************************/ 
            while(messageBuffer[tokenizer] != '\r')
            {
            	host[i] = messageBuffer[tokenizer]; 
            	tokenizer++; 
            	i++; 
            }

            host[i] = '\0'; 
            tokenizer++; 
            i=0; 

            if(DEBUG)
            {
            	printf("\nHTTP Request Type: %s", httpOperation); 
				printf("\nHTTP version: %s", httpVersion); 
				printf("\nPath to file: %s", pathToFile); 
				printf("\nHost: %s", host); 
            }
            

            /************************************************/ 
           	/* Examine the first character of the request 	*/ 
           	/* to determine what HTTP operation it is 		*/ 
           	/************************************************/ 
           	switch(messageBuffer[0])
           	{
           		case 'G': 
           			if(DEBUG)
           				printf("\nReceived HTTP GET request."); 
           			break; 
           		case 'P': 
           			if(DEBUG)
           				printf("\nReceived HTTP POST request."); 
           			break; 
           		case 'H':
           			if(DEBUG)
           				printf("\nReceived HTTP HEAD request."); 
           			break; 
           		default: 
           			break; 
           	}

            if(DEBUG)
           		printf("\nReceived %zu bytes from the client. Here is the message I received: \n\n%s\n", numBytes, messageBuffer);

           	/****************************************/ 
           	/* Send some data back to the client 	*/ 
           	/****************************************/ 
           	numBytes = send(clntSock, messageBuffer, strlen(messageBuffer), 0); 

           	if(numBytes < 0)
            	DieWithSystemMessage("send() failed\n"); 
            else if(numBytes == 0)
            {
            	if(DEBUG)
            		printf("\nsend() failed: No data sent."); 

            	continue;		// go back to the beginning of the infinite loop  
            }



           	if(DEBUG)
           		printf("\nSuccessfully sent %zu bytes to client on socket #%d. Here is the message I sent: \n\n%s\n", numBytes, clntSock, messageBuffer); 




        }

		/****************************************************************/ 
		/* Fork a process to handle communication for this request 		*/ 
		/****************************************************************/ 
		pid = fork(); 
		if(pid == 0)
		{
			if(DEBUG)
			{
				printf("\nSuccessfully forked process %d to handle communication with %s", pid, pathToFile); 
			}
			close(sock); 
			do
			{
				// Do stuff as long as we're talking to the client 
				// (1) Read an HTTP request from clntSock 
				// (2) Create a new socket connected to the server specified in the client's HTTP request 
				// (3) Pass an optionally-modified version of the client's request and send it to the server 
				// (4) Read the server's response message and pass an optionally-modified version of it back to the client 
			}
			while(1); 

			close(clntSock); 
		}
		else if(pid < 0)
		{
			if(DEBUG)
			{
				printf("\nFailed to fork process to handle communication with %s", pathToFile); 
			}
		}

		printf("\npid=%d", pid); 

		close(clntSock); 
		childCount++; 

		while(childCount)
		{
			pid = waitpid(-1, NULL, WNOHANG); 
			if(pid == 0)
				break; 
			else
				childCount--; 
		}
		
	}
	while(1); 



	printf("\nPress any key to exit..."); 
	char ch = getchar(); 
	return 0; 
}