#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include "DieWithMessage.c"
#include "debug_config.c"		// contains the DEBUG flag
#include "hostname_helpers.c" 	// helper functions pertaining to host names 

#define PORT 80 

int main()
{
	int clntSock; 
	struct sockaddr_in clntAddr; 		// address structure for this client 

	char host[100] = "www.griffinfujioka.com"; 
	char ip[100];// = "192.168.0.11\0"; 

	hostname_to_ip(host, ip); 

	clntSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 

	if(clntSock < 0)
	{
		printf("\nsocket() failed"); 
		return 0; 
	}

	struct sockaddr_in servAddr; 
    memset(&servAddr, 0, sizeof(servAddr)); 
    servAddr.sin_family = AF_INET;                           	// IPV4 address family 
    servAddr.sin_port = htons(PORT); 							// Port to be bound to 

    int rtnVal = inet_pton(AF_INET, ip, &servAddr.sin_addr.s_addr); 

    if(rtnVal == 0)
    {
    	printf("\ninet_pton() failed. Invalid address string: %s", host); 
    	return 0; 
    }
    else if(rtnVal < 0)
    {
    	printf("\ninet_pton() failed.");
    	return 0; 
    }

    if(connect(clntSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
    {
    	printf("\nconnect() failed"); 
    }
    else
    {
    	printf("\nconnect() succeeded"); 
    }

    close(clntSock);

    printf("\n\n"); 



}