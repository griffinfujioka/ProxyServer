int hostname_to_ip(char* host, char* ip)
{
	int sockfd; 
	struct addrinfo hints, *servInfo, *p; 
	struct sockaddr_in *h;
	int rv; 

	if(DEBUG)
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
		if(DEBUG)
			printf("\nResolved host name %s to IP address %s", host, inet_ntoa(h->sin_addr)); 
	}

	freeaddrinfo(servInfo); 

	return 0; 

}

void build_hostname(char* host, char* fixedHostname)
{
	int i = 0; 

	if(strncmp(host, "www.", 4) == 0)
	{
		strncpy(fixedHostname, host, strlen(host)); 
		return; 
	}


	for(i=0; i < 3; i++)
	{
		fixedHostname[i] = 'w'; 
	}

	fixedHostname[i] = '.'; 
	i++; 

	int j = 0; 
	while(host[j] != 0)
	{
		fixedHostname[i] = host[j]; 
		i++; 
		j++; 
	}

	fixedHostname[i] = '\0'; 	
	if(DEBUG)
		printf("\nBuilt host name: %s", fixedHostname); 
}