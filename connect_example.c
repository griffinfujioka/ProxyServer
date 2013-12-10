#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include "DieWithMessage.c"

#define PORT 80 

int hostname_to_ip(char* host, char* ip)
{
	int sockfd; 
	struct addrinfo hints, *servInfo, *p; 
	struct sockaddr_in *h;
	int rv; 


	printf("\nAttempting to resolve host name %s", host); 

	memset(&hints, 0, sizeof(hints)); 
	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_STREAM; 


	if((rv = getaddrinfo(host, "http", &hints, &servInfo)) != 0)
	{
		printf("\nError"); 
		return 1; 
	}

	for(p=servInfo; p != NULL; p = p->ai_next)
	{
		h = (struct sockaddr_in *)p->ai_addr;
		strcpy(ip, inet_ntoa(h->sin_addr)); 

		printf("\nResolved host name %s to IP address %s", host, inet_ntoa(h->sin_addr)); 
		return 0; 
	}

	freeaddrinfo(servInfo); 

	return 0; 

}

int main()
{
	int clntSock; 
	struct sockaddr_in clntAddr; 		// address structure for this client 

	char google[100] = "www.google.com\0"; 
	char ip[100];// = "192.168.0.11\0"; 

	hostname_to_ip(google, ip); 

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
    	printf("\ninet_pton() failed. Invalid address string"); 
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