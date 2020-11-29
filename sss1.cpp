// Server program 
#include <arpa/inet.h> 
#include <errno.h> 
#include <netinet/in.h> 
#include <signal.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <strings.h> 
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <fcntl.h>
#define PORT 5000 
#define MAXLINE 1024 


int max(int x, int y) 
{ 
	if (x > y) 
		return x; 
	else
		return y; 
} 
int main() 
{ 
	//client variables
	fd_set rset1;
	int sockfd; 
	char buffer[MAXLINE];  
	struct sockaddr_in servaddr1; 
	 
	// Creating socket file descriptor 
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
		printf("socket creation failed"); 
		exit(0); 
	} 

	//server variables
	int listenfd, connfd, udpfd, nready, maxfdp1, client_sockets[3], connection_index=0;  
	pid_t childpid; 
	fd_set rset; 
	ssize_t n; 
	socklen_t len; 
	const int on = 1; 
	struct sockaddr_in cliaddr, servaddr; 
	char message[MAXLINE] = "Hello Client"; 
	void sig_chld(int); 

	/* creting TCP socket to listen to server */ 
	servaddr1.sin_family = AF_INET; 
	servaddr1.sin_port = htons(5001); 
	servaddr1.sin_addr.s_addr = inet_addr("127.0.0.1"); 

	if (connect(sockfd, (struct sockaddr*)&servaddr1, 
							sizeof(servaddr1)) < 0) { 
		printf("\n Error : Connect Failed \n"); 
	} 

	int readflags = fcntl(sockfd, F_GETFL, 0), temp;
	readflags = fcntl(sockfd, F_SETFL, readflags | O_NONBLOCK);
	while(temp = read(sockfd, buffer, sizeof(buffer)) < 0)
	{
	}
	printf("server: "); 
	puts(buffer);
	bzero(buffer,sizeof(buffer));
	
	
	/* create listening TCP socket */
	listenfd = socket(AF_INET, SOCK_STREAM, 0); 
	bzero(&servaddr, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT); 

	// binding server addr structure to listenfd 
	bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)); 
	listen(listenfd, 10); 

	
	// clear the descriptor set 

	// get maxfd 
	maxfdp1 = max(listenfd, udpfd) + 1; 
	for (;;) { 

		FD_ZERO(&rset); 

		// set listenfd and udpfd in readset 
		FD_SET(listenfd, &rset); 
		FD_SET(udpfd, &rset); 

		for(int i=0;i<connection_index;i++)
		{
			if(client_sockets[i]>0)
				FD_SET(client_sockets[i],&rset);
			if(client_sockets[i] > maxfdp1);
				maxfdp1 = client_sockets[i] + 1;
		}
		
		// select the ready descriptor 
		nready = select(maxfdp1, &rset, NULL, NULL, NULL); 

		// if tcp socket is readable then handle 
		// it by accepting the connection 
		if (FD_ISSET(listenfd, &rset)) 
		{ 
		
			len = sizeof(cliaddr); 
			connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len); 
		
			bzero(buffer, sizeof(buffer)); 
			printf("Connected to client: %d\n", cliaddr.sin_port); 
			
			write(connfd, (const char*)message, sizeof(message)); 

			client_sockets[connection_index++] = connfd;		 
		}
		
		
		for(int i = 0; i < connection_index; i++)
		{
			if(FD_ISSET(client_sockets[i], &rset))
			{
				bzero(buffer, sizeof(buffer)); 
				printf("Client %d: ", client_sockets[i]); 
				read(client_sockets[i], buffer, sizeof(buffer)); 
				puts(buffer);
				printf("Message to Client: ");
				scanf("%[^\n]%*c", buffer); 
				write(client_sockets[i], buffer, sizeof(buffer));
			}
		}
		// if udp socket is readable receive the message. 
		if (FD_ISSET(udpfd, &rset)) { 
			len = sizeof(cliaddr); 
			bzero(buffer, sizeof(buffer)); 
			printf("\nMessage from UDP client: "); 
			n = recvfrom(udpfd, buffer, sizeof(buffer), 0, 
						(struct sockaddr*)&cliaddr, &len); 
			puts(buffer); 
			scanf("%[^\n]%*c", buffer); 

			sendto(udpfd, buffer, sizeof(buffer), 0, 
				(struct sockaddr*)&cliaddr, sizeof(cliaddr)); 
		} 
		
		/*  receiving from server */
		if(temp = read(sockfd, buffer, sizeof(buffer)) > 0)
		{
			printf("server: "); 
			puts(buffer);
			bzero(buffer,sizeof(buffer));

		}
		printf("Message to server: ");
		scanf("%[^\n]%*c",buffer);
		write(sockfd, buffer, sizeof(buffer));
		//checking if to commit
	} 
}
