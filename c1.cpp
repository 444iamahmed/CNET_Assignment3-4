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
	string request, message, ans;
	int sockfd, clientno, choice1 ,choice2, dest_port; 
	//char buffer[MAXLINE], ans; 
	//char* message; 
	struct sockaddr_in my_addr, servaddr; 
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
	cout<<"The Servers are: \n 5001 \n 5002 \n 5004 \n"; 
    	cout<<"ENTER SERVER TO CONNECT TO: \n";
	cin>>servaddr.sin_port;
	
	if( servaddr.sin_port != 5001 && servaddr.sin_port != 5002 && servaddr.sin_port != 5004 )
	{
		cout<<"Incorrect port try again"<<endl;
		cin>>servaddr.sin_port;
	}
	servaddr.sin_port=htons(servaddr.sin_port);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

	if (connect(sockfd, (struct sockaddr*)&servaddr, 
							sizeof(servaddr)) < 0) { 
		cout<<"\n Error : Connect Failed \n"; 
	} 
	
	else
	{
		bzero(&my_addr, sizeof(my_addr));
		len = sizeof(my_addr);
		getsockname(sockfd, (struct sockaddr *) &my_addr, &len);

		int readflags = fcntl(sockfd, F_GETFL, 0), temp;
		readflags = fcntl(sockfd, F_SETFL, readflags | O_NONBLOCK);
		

		for(;;)
		{
			
			cout<<"Enter \n 1: To send DNS request \n 2: To connect to client \n";
			cin>>choice1;
			if(choice1 == 1)
			{
				packet _packet = dns_req;
				if(temp = read(sockfd, &_packet, sizeof(_packet)) > 0)
				{
					if(_packet.type == dns_req)
					{
						cout<<_packet._dns_request.response<<"FOR "<<_packet._dns_request.request<<"\nROUTE: ";
						for(int i = 0; i < _packet._dns_request.num_ports_visited; ++i)
							cout<<_packet._dns_request.ports_visited[i]<<" -> ";
						cout<<"\n";
					}

				}
				else
				{

					cout<<"ENTER DNS REQUEST: \n";
					cin>>request;
					_packet = packet(request, my_addr.sin_port);
					write(sockfd, &_packet, sizeof(packet));
					sleep(3);
				}
			}
			else
			{
				cout<<"Enter \n 1: if you want to connect to a client \n 2: if you want to see any pending message"<<endl<<endl;
				cin>>choice2;
				packet _packet;
				if(choice2 == 1)
				{
					cout<<"Enter client port to connect to: ";
					cin>>dest_port;
					while(strcmp(_packet.message,"close"))
					{
						cout<<"Enter message: "<<endl;
						cin.ignore();
						getline(cin, message);

						_packet = packet(message, dest_port, my_addr.sin_port); 
						write(sockfd, &_packet, sizeof(_packet));						
					
						if(!strcmp(_packet.message ,"close"))
							break;
						
						cout<<"Want to send another message or wait for reply(reply) or CLOSE(close)"<<endl;
						cin.ignore();
						getline(cin, message);
						
						if(ans=="reply")
						{
							cout<<"recahing"<<endl;
							temp = read(sockfd, &_packet, sizeof(_packet));
							swap(_packet.dest_port, _packet.source_port);
							cout<<"Message: "<<_packet.message<<endl;					
						}
						if(temp = read(sockfd, &_packet, sizeof(_packet)) > 0)
						{
							cout<<"You have a new message: "<<_packet.message<<endl;
							swap(_packet.dest_port, _packet.source_port);
						}
						if(ans=="close")
						{
							strcpy(_packet.message, ans.c_str());
							write(sockfd, &_packet, sizeof(packet));
						}			
					}
				}
				else
				{
					if(temp = read(sockfd, &_packet, sizeof(_packet)) > 0)
					{
						cout<<"Message: "<<_packet.message<<endl;
						cout<<"Send a reply or enter close to end connection"<<endl;
						while(strcmp(_packet.message,"close"))
						{
							cin.ignore();
							getline(cin, message); 
							swap(_packet.dest_port, _packet.source_port);
							strcpy(_packet.message, message.c_str());
							write(sockfd, &_packet, sizeof(packet));
						
							if(!strcmp(_packet.message,"close"))						
								break;
									
							cout<<endl<<endl;
							cout<<"Want to send another message or wait for reply(r) or CLOSE"<<endl;
							cin>>ans;
							if(ans[0] == 'r')
							{cout<<"recahing1"<<endl;
								temp = read(sockfd, &_packet, sizeof(_packet));
								cout<<"Message: "<<_packet.message<<endl;					
							}
							if(ans=="close")
							{
								strcpy(_packet.message, ans.c_str());
								write(sockfd, &_packet, sizeof(packet));
								break;
							}						
							cout<<"Enter message: "<<endl;		
						}
					}
					else
					{	
						cout<<"No new message"<<endl;		
					}
				}
				bzero(_packet.message,sizeof(_packet.message));
			}
		 
	}
	close(sockfd);
	} 
} 
