// TCP Client program 
#include <netinet/in.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <fcntl.h>
#include<iostream>
#include "packet.h"
#define MAXLINE 1024
using namespace std; 
int main() 
{ 
	fd_set rset;
	int sockfd, clientno; 
	char buffer[MAXLINE]; 
	char* message; 
	struct sockaddr_in servaddr; 
	packet _packet;
	int n, len; 
	// Creating socket file descriptor 
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
		printf("socket creation failed"); 
		exit(0); 
	} 

	memset(&servaddr, 0, sizeof(servaddr)); 

	// Filling server information 
	servaddr.sin_family = AF_INET; 
    	cout<<"ENTER SERVER TO CONNECT TO: \n";
	cin>>servaddr.sin_port; 
	servaddr.sin_port=htons(servaddr.sin_port);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

	if (connect(sockfd, (struct sockaddr*)&servaddr, 
							sizeof(servaddr)) < 0) { 
		printf("\n Error : Connect Failed \n"); 
	} 

	int readflags = fcntl(sockfd, F_GETFL, 0), temp;
	readflags = fcntl(sockfd, F_SETFL, readflags | O_NONBLOCK);
	/*while(temp = read(sockfd, buffer, sizeof(buffer)) < 0)
	{
		printf("server: "); 
		puts(buffer);
	}*/
	bzero(buffer,sizeof(buffer));

	for(;;)
	{
		
		
		memset(buffer, 0, sizeof(buffer)); 

		if(temp = read(sockfd, buffer, sizeof(buffer)) > 0)
		{
			printf("server: "); 
			puts(buffer);
			bzero(buffer,sizeof(buffer));

		}
		else
		{
			printf("Enter the number of client to send message to: ");
			cin>>_packet.message.dest_port;
			cout<<"Enter message"<<endl;
			cin>>_packet.message.text;
			write(sockfd, &_packet, sizeof(packet));
			cout<<"Message sent"<<endl;
		}
		read(sockfd, buffer, sizeof(buffer));
		 
	}
	close(sockfd); 
} 
