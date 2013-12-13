#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include "DieWithMessage.c"
#include "debug_config.c"		// contains the DEBUG flag
#include "hostname_helpers.c" 	// helper functions pertaining to host names 

static int SIZEOF_MESSAGEBUFFER = 1024 * 1024;

static int MAXPENDING = 5; 

static int SIZEOF_HTTP_OPERATION = 5; 

static int SIZEOF_PATH_TO_FILE = 256; 

static int SIZEOF_HTTP_VERSION = 24; 

static int SIZEOF_HOSTNAME = 256; 

static int SIZEOF_IPADDRESS = 4; 

#define PORT 2500 

void HandleHttpRequest(char* messageBuffer, char* httpOperation, char* pathToFile, char* httpVersion, char* host); 

void CreateModifiedHttpRequest(char* originalMessage, char* modifiedMessage); 


int main()
{
	char messageBuffer[SIZEOF_MESSAGEBUFFER]; 
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

	int serverSock = 0; 			// socket for connecting to server 

	struct sockaddr_in clntAddr; 		// client address
	socklen_t clntAddrLen; 

	int haveDeterminedContentLength = 0; 
	int contentLength = 0; 

	char ip[SIZEOF_IPADDRESS];				// buffer for storing server's IP address 										
	char fixedHostname[SIZEOF_HOSTNAME]; 	// buffer for "fixed" version of host name so that getaddrinfo() will work properly 

	int contentTypeIsNotImplemented = 0; 		// Used to re-direct the program if we see a Content-Type we aren't prepared for 

	printf("================================================="); 
	printf("\nWelcome to the CptS 455 Proxy Server!"); 
	printf("\n\tCreated by: Griffin Fujioka"); 
	printf("\n\tNovember, 2013"); 
	printf("\n=================================================\n\n"); 

	/************************************************************/ 
	/* Create a socket for handling new connection requests 	*/ 
	/* This is the socket which will be used 
	/************************************************************/ 
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)			// sock = s1 
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


	// while still serving... 
	do
	{
		contentTypeIsNotImplemented = 0; 
		iterationCounter++; 
		printf("\nIteration %d: \n\n", iterationCounter); 
		if(DEBUG)
			printf("\nServer ready for incoming connections... Incoming connections will be accepted through socket #%d", sock); 

		/****************************************/
		/* Zero out all important buffers 		*/
		/****************************************/  
		memset(&messageBuffer, 0, SIZEOF_MESSAGEBUFFER); 			// zero the message buffer 
		memset(&httpOperation, 0, SIZEOF_HTTP_OPERATION); 		// zero the HTTP operation buffer
		memset(&pathToFile, 0, SIZEOF_PATH_TO_FILE); 			// zero the path to file buffer 
		memset(&httpVersion, 0, SIZEOF_HTTP_VERSION); 			// zero the HTTP version buffer
		memset(&host, 0, SIZEOF_HOSTNAME); 						// zero the host buffer 
		memset(&clntAddr, 0, sizeof(clntAddr)); 				// zero the client address buffer 

		

		clntAddrLen = sizeof(clntAddr);	

		/************************************************************/ 
		/* Accept connections from web clients on PORT				*/ 
		/* 		i.e., a web browser 								*/ 
		/************************************************************/ 
		int clntSock = accept(sock, (struct sockaddr*) &clntAddr, &clntAddrLen); 			// clntSock = s2 

		if(clntSock < 0)
		{
			perror("accept"); 
			break;
		}
		else
		{
			

			if(DEBUG)
				printf("\nSuccessfully accepted connection. Client is using socket %d", clntSock); 

			char clntName[INET_ADDRSTRLEN];                 // String for client address
			

	        if(inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName, sizeof(clntName)) != 0)
	        {
	        	printf("\nHandling client on socket #%d: \n\tIP address: %s\n\tPort #: %d\n", clntSock, clntName, clntAddr.sin_port);


				/****************************************************************/ 
				/* Fork a process to handle communication for this request 		*/ 
				/****************************************************************/ 
				pid = fork(); 

				if(pid >= 0)
				{

					//close(sock);  			// closing fd does not close socket unless last reference, which occurs after we stop serving 

					// do-while communicating with the client...
					do
					{ 
						/********************************************************/ 
			        	/* (1) Receive and read an HTTP request from the client */ 
			        	/********************************************************/ 


			        	/****************************************************************/ 
			        	/* Question: Do I need to extract the filename, document root 	*/ 
			        	/* and extension type to assign it to the Content-Type header? 	*/ 
			        	/* Do I need to update the status header to 200? 				*/ 
			        	/****************************************************************/ 

			        	/****************************************************************/ 
			        	/* Do I need to send the status & content type separately 		*/ 
			        	/* from the body? 												*/ 
			        	/****************************************************************/ 
			        	numBytes = recv(clntSock, messageBuffer, SIZEOF_MESSAGEBUFFER, 0);



			        	if(numBytes < 0)
			            	DieWithSystemMessage("recv() failed\n"); 
			            else if(numBytes == 0)
			            {
			            	if(DEBUG)
			            		printf("\nrecv() failed: No data received."); 

			            	break;		// go back to the beginning of the infinite loop  
			            }

			            if(DEBUG)
		           			printf("\nReceived %zu bytes from the client. Here is the message I received: \n\n%s\n", numBytes, messageBuffer);


		           		HandleHttpRequest(messageBuffer, httpOperation, pathToFile, httpVersion, host); 

		           		memset(&ip, 0, sizeof(ip));
						memset(&fixedHostname, 0, sizeof(fixedHostname));

						// Build a proper host name so the hostname_to_ip() call below will work properly 
						build_hostname(host, fixedHostname); 

						// Convert the host name to it's IP address, store in ip 
						hostname_to_ip(fixedHostname, ip); 

		           		if(DEBUG)
						{
							printf("\nSuccessfully forked process %d to handle communication with client %s", pid, fixedHostname); 
							printf("\nClosed socket #%d in the newly forked process, but it is still alive in it's original process.", sock); 
							printf("\n"); 
						}

		           		char modifiedMessage[SIZEOF_MESSAGEBUFFER]; 
		           		memset(modifiedMessage, 0, sizeof(modifiedMessage)); 

		           		//CreateModifiedHttpRequest(messageBuffer, modifiedMessage); 



		           		if(DEBUG)
		           		{
		           			printf("\nHost name of server: %s", fixedHostname); 
		           			printf("\nModified HTTP request: \n%s", modifiedMessage); 
		           		}

		           		/********************************************************************************************/ 
						/* (2) Create a new socket connected to the server specified in the client's HTTP request 	*/ 
						/********************************************************************************************/ 
						if((serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)			
		    				DieWithSystemMessage("socket() failed"); 


						struct sockaddr_in serverAddr; 			// server address structure
						serverAddr.sin_family = AF_INET; 
						serverAddr.sin_port = htons(80); 		// We don't know what port to use, try default HTTP port 


						// Convert the IP address to network byte order and store it in the serverAddr structure 
						inet_pton(AF_INET, ip, &serverAddr.sin_addr.s_addr); 


						if(DEBUG)
						{
							printf("\nServer's IP address: %s", ip); 
							printf("\nAttempting to connect() to server using \n\tSocket #%d\n\tIP address %s \n\tPort #%d", 
								serverSock, ip, htons(clntAddr.sin_port)); 
						}

						if((connect(serverSock, (struct sockaddr *) &serverAddr, sizeof(serverAddr))) < 0)
						{
							if(DEBUG)
							{
								printf("\nconnect() failed: failed to connect to server"); 
								continue; 
							}
						}

						if(DEBUG)
						{
							printf("\nSuccessfully connected to host %s!", fixedHostname); 
						}


						/************************************************************************************************/ 
						/* (3) Pass an optionally-modified version of the client's request and send it to the server 	*/   
						/************************************************************************************************/ 

						if(DEBUG)
							printf("\nI will now pass the request to the server %s using socket #%d", fixedHostname, serverSock); 


			           	numBytes = send(serverSock, messageBuffer, strlen(messageBuffer), 0); 

			           	if(numBytes < 0)
			            	DieWithSystemMessage("send() failed\n"); 
			            else if(numBytes == 0)
			            {
			            	if(DEBUG)
			            		printf("\nsend() failed: No data sent."); 

			            	continue;		// go back to the beginning of the infinite loop  
			            }



			           	if(DEBUG)
			           	{


			           		printf("\nSuccessfully passed %zu bytes to server on socket #%d. Here is the message I sent: \n\n%s\n", 
			           			numBytes, serverSock, messageBuffer); 
			           		printf("\nI am now waiting for the server's response..."); 
			           	}

			           	ssize_t totalBytesRecvd = 0; 

		           		/************************************************************************************************************/ 
						/* (4) Read the server's response message and pass an optionally-modified version of it back to the client 	*/ 
						/************************************************************************************************************/ 
						while( numBytes >= 0)
						{

							memset(messageBuffer, 0, SIZEOF_MESSAGEBUFFER); 			// zero the buffer and re-use it 
							numBytes = recv(serverSock, messageBuffer, SIZEOF_MESSAGEBUFFER, 0);
							totalBytesRecvd += numBytes; 

				        	if(numBytes < 0)
				        	{
				            	printf("recv() failed\n"); 
				            	break; 
				            }
				            else if(numBytes == 0)
				            {
				            	if(DEBUG)
				            		printf("\nrecv() failed: No data received."); 

				            	break;		// go back to the beginning of the infinite loop  
				            }

				     //        int c = 0; 
				     //        while(c < strlen(messageBuffer))
				     //        {
				     //        	if((strncmp(&messageBuffer[c], "Content-Type: ", strlen("Content-Type")) == 0))
				     //        	{
				     //        		while(messageBuffer[c] != ' ')
				     //        		{
				     //        			c++; 
				     //        		}
				     //        		c++; 

				     //        		char contentTypeBuff[20]; 
				     //        		int count = 0; 

				     //        		printf("\n"); 

									// while(messageBuffer[c] != '\r' && c < strlen(messageBuffer))
									// {
									// 	contentTypeBuff[count] = messageBuffer[c]; 
									// 	count++; 
									// 	c++; 
									// }

									// contentTypeBuff[count++] = '\0'; 

									// if(DEBUG)
									// {
									// 	printf("\nContent-Type is %s", contentTypeBuff); 
									// }

									// // if(strncmp(contentTypeBuff, "image", strlen("image")) == 0)
									// // {
									// // 	if(DEBUG)
									// // 	{
									// // 		printf("\nDetermined that Content-Type is image/*. Daddy don't play like dat, aaand I'm out."); 
									// // 		contentTypeIsNotImplemented = 1; 
									// // 		break; 
									// // 	}
									// // }
									// break; 
					    //         }
					            
					    //         c++; 
				     //        }

				     //        if(contentTypeIsNotImplemented)
				     //        	break; 


				            // If this is the server's first response message to us, 
				            // examine the response header's to determine the Content-Length. 
				            // That is, the number of bytes the server is about to send. 
				            if(!haveDeterminedContentLength)
				            {
								int tokenizer = 0; 
								

								while(strncmp(&messageBuffer[tokenizer], "Content-Length: ", strlen("Content-Length: ")) != 0 && 
									tokenizer < strlen(messageBuffer))
								{

									tokenizer++; 
								}

								while(messageBuffer[tokenizer] != ' ')
									tokenizer++; 

								tokenizer++; 

								char contentLengthBuff[20]; 
								int count = 0; 

								printf("\n"); 

								while(messageBuffer[tokenizer] != '\r' && tokenizer < strlen(messageBuffer))
								{
									contentLengthBuff[count] = messageBuffer[tokenizer]; 
									count++; 
									tokenizer++; 
								}

								contentLengthBuff[count++] = '\0'; 

								

								
								if(contentLengthBuff[0] != 0)
								{
									contentLength = atoi(contentLengthBuff); 
									haveDeterminedContentLength = 1; 

									if(DEBUG)
										printf("\nPer the server's response message, I'm expecting a Content-Length of %d bytes.", contentLength); 
								}
							}

							



				            if(DEBUG)
				            {
				       			printf("\nReceived %zu bytes from the server. Here is the response message I received: \n\n%s\n", 
				       				numBytes, messageBuffer);
				       			printf("\nNow I will pass the server's response back the client on socket #%d", clntSock); 
				       			//continue; 
				       		}

				       		numBytes = send(clntSock, messageBuffer, numBytes,0); 

				       		if(numBytes < 0)
				            	DieWithSystemMessage("send() failed\n"); 
				            else if(numBytes == 0)
				            {
				            	if(DEBUG)
				            		printf("\nsend() failed: No data sent."); 

				            	continue;		// go back to the beginning of the infinite loop  
				            }

				            if(totalBytesRecvd >= contentLength && haveDeterminedContentLength)
							{
								printf("\nI've received all of the data I was expecting to receive from the server. I was expecting %d bytes and I've received %zu", 
									contentLength, totalBytesRecvd); 
								break; 
							}
							else if(totalBytesRecvd > 0 && !haveDeterminedContentLength)
							{
								printf("\nDid not detect Content-Length header in server response. What gives!?"); 
								break; 
							}



				           	if(DEBUG)
				           	{
				           		//printf("\nSuccessfully passed %zu bytes to client on socket #%d. Here is the message I sent: \n\n%s\n", numBytes, clntSock, messageBuffer); 

								printf("\nI have received %zu / %d expected bytes...", totalBytesRecvd, contentLength); 
							
				           	}
		           		}

					} while(1); 

					close(clntSock); 

					if(DEBUG)
						printf("\nDone communicating with %s. \nSuccessfully closed socket %d.", pathToFile, clntSock); 
				}
				else if(pid < 0)
				{
					if(DEBUG)
					{
						printf("\nFailed to fork process to handle communication with %s", pathToFile); 
					}
				}

				close(clntSock); 
				childCount++; 


				// Kill off some child processes
				while(childCount)
				{
					pid = waitpid(-1, NULL, WNOHANG); 
					if(pid == 0)
						break; 
					else
					{
						childCount--; 
						if(DEBUG)
						{
							printf("\nKilled process %d", pid); 
						}
					}
				}
			}

			haveDeterminedContentLength = 0; 

			printf("\n=============================================\n"); 

		}

		//close(sock); 
		
	}
	while(1); 

	close(sock); 


	printf("\nPress any key to exit..."); 
	char ch = getchar(); 
	return 0; 
}


/****************************************************************************/ 
/* Upon receiving an HTTP request message into messageBuffer, parse the 	*/ 
/* HTTP request and populate the httpOperation, pathToFile, httpVersion 	*/ 
/* and host buffers. 														*/ 
/****************************************************************************/ 
void HandleHttpRequest(char* messageBuffer, char* httpOperation, char* pathToFile, char* httpVersion, char* host)
{
	if(DEBUG)
	{
		printf("\nProcessing HTTP request... it has %lu characters. \n", strlen(messageBuffer));
	}

	int count = 0; 				// used to iterate all the way through messageBuffer
    int buff = 0; 				// used for filling up respective buffers with information from the HTTP request headers
    int index = 0; 				// used for iterating through individual lines 
    int tok = 0; 				// used to tokenize individual lines 
    int mod = 0; 				// used for populating the modified message 

    
	/********************************************************/ 
	/* Parse messageBuffer to attain the HTTP operation 	*/ 
    /********************************************************/  
    while(messageBuffer[tok] != ' ')
    {
    	httpOperation[buff] = messageBuffer[tok]; 
    	tok++; 
    	buff++; 
    }

    httpOperation[buff] = '\0'; 
    tok++; 
    buff=0; 

    /********************************************************/ 
    /* Parse messageBuffer to attain the path to the file 	*/ 
    /********************************************************/ 
    while(messageBuffer[tok] != ' ')
    { 
    	pathToFile[buff] = messageBuffer[tok]; 
    	tok++; 
    	buff++; 
    }

    pathToFile[buff] = '\0'; 
    tok++; 
    buff=0; 

    /********************************************************/ 
    /* Parse messageBuffer to attain the HTTP version 		*/ 
    /********************************************************/ 
    while(messageBuffer[tok] != '\r')
    { 
    	httpVersion[buff] = messageBuffer[tok]; 
    	tok++; 
    	buff++; 
    }

    httpVersion[buff] = '\0'; 
    tok++; 

    count += tok; 
    buff=0;  

    // Now reading line 2 which tells us the host 
    /********************************************************/ 
    /* Parse messageBuffer to attain host name 				*/ 
    /********************************************************/ 
    // Skip over the 'Host: ' part 
    while(messageBuffer[tok] != ' ')		
    {
    	tok++;
    }

    tok++; 

    while(messageBuffer[tok] != '\r')
	{
		host[buff] = messageBuffer[tok]; 
    	tok++; 
    	buff++; 
	}

    host[buff] = '\0'; 
	tok++; 


	// Done reading line 2 
	count += tok; 


    if(DEBUG)
    {
    	printf("\n============================="); 
    	printf("\n      HTTP HEADERS           ");
    	printf("\n============================="); 
    	printf("\nHTTP Request Type: %s", httpOperation); 
		printf("\nHTTP version: %s", httpVersion); 
		printf("\nPath to file: %s", pathToFile); 
		printf("\nHost: %s", host); 
		printf("\n\n"); 
    }
}

void CreateModifiedHttpRequest(char* originalMessage, char* modifiedMessage)
{
	int count = 0, i = 0; 
	char line[128]; 

	while(count <= strlen(originalMessage)+1)
	{
		memset(line, 0, sizeof(line));

		if(strncmp(&originalMessage[count], "\r\n\r\n", 4) == 0)
		{
			printf("\nFound empty line! That means we've reached the end of the HEADERS!"); 
			break; 
		}

		i=0; 

		while(originalMessage[count] != '\r')
		{
			line[i] = originalMessage[count]; 
			count++; 
			i++; 
		}

		line[i] = '\r'; 
		line[i+1] = '\n'; 

		count += 2;  		// skip over \n and check out the next line 

		if(strncmp(line, "Proxy-Connection", strlen("Proxy-Connection")) == 0)
		{
			// Skip the Proxy-Connection header 
			printf("\nFound and removed Proxy-Connection header.\n\n"); 
			continue; 
		}
		else if(strncmp(line, "Connection", strlen("Connection")) == 0)
		{
			printf("\nFound and removed Connection header."); 
			continue; 
		}

		if(line[0] == '\r')
		{
			printf("\nFound the blank line!");

			break; 
		}


		strcat(modifiedMessage, line); 

		printf("Line: %s", line); 


	}

	// Insert Connection: close header into modified message 
	//printf("\nmodifiedMessage[count] : %c", modifiedMessage[count]); 

	// memset(line, 0, sizeof(line)); 
	// strcat(line, "Connection: close\r\n"); 
	// strcat(modifiedMessage, line); 

	// // Insert \r\n\r\n to denote the end of the HEADERS section 
	// memset(line, 0, sizeof(line)); 
	// strcat(line, "\r\n\r\n"); 
	// strcat(modifiedMessage, line); 


	//printf("\nModified message: \n\n%s", modifiedMessage); 

}



