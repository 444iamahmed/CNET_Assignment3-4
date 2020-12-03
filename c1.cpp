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
	string request;
	int sockfd, clientno; 
	//char buffer[MAXLINE], ans; 
	//char* message; 
	struct sockaddr_in my_addr, servaddr; 
	packet _packet;
	int n;
	socklen_t len; 
	// Creating socket file descriptor 
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
		cout<<"socket creation failed"; 
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
		cout<<"\n Error : Connect Failed \n"; 
	} 

	bzero(&my_addr, sizeof(my_addr));
	len = sizeof(my_addr);
	getsockname(sockfd, (struct sockaddr *) &my_addr, &len);

	int readflags = fcntl(sockfd, F_GETFL, 0), temp;
	readflags = fcntl(sockfd, F_SETFL, readflags | O_NONBLOCK);
	

	for(;;)
	{
		
		//cout<<"do you want to send message (y/n)"<<endl;
		//cin>> ans;
		
		if(temp = read(sockfd, &_packet, sizeof(_packet)) > 0)
		{
			if(_packet.type == dns_req)
			{
				cout<<_packet._dns_request.response<<"FOR "<<_packet._dns_request.request<<"\nROUTE: ";
				for(int i = 0; i < _packet._dns_request.num_ports_visited; ++i)
					cout<<_packet._dns_request.ports_visited[i]<<" -> ";
				cout<<"\n";
			}

			//cout<<"Message From Host: "<<_packet.dns<<endl;

		}
		else
		{

			/*cout<<"Enter the number of client to send message to: ";
			cin>>_packet.dest_port;
			cout<<"Enter message"<<endl;
			cin>>_packet.message;
			cout<<write(sockfd, &_packet, sizeof(packet))<<"\n";
			cout<<"Message sent"<<endl;*/
			cout<<"ENTER DNS REQUEST: \n";
			cin>>request;
			_packet = packet(request, my_addr.sin_port);
			write(sockfd, &_packet, sizeof(packet));
			sleep(3);
		}
		 
	}
	close(sockfd); 
} 
