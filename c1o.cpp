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
	int sockfd, clientno, choice;
	string ans; 
	char buffer[MAXLINE]; 
	char* message; 
	struct sockaddr_in servaddr; 
	packet _packet(msg);
	int n, len; 
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
	
	else
	{
	int readflags = fcntl(sockfd, F_GETFL, 0), temp;
	readflags = fcntl(sockfd, F_SETFL, readflags | O_NONBLOCK);

	for(;;)
	{
	
		cout<<"Enter \n 1: if you want to connect to a client \n 2: if you want to see any pending message"<<endl<<endl;
		cin>>choice;
		
		if(choice==1)
		{
			cout<<"Enter client port to connect to: ";
			cin>>_packet.message.dest_port;
			while(strcmp(_packet.message.text,"close"))
			{
				cout<<"Enter message: "<<endl;
				cin.ignore();
				cin.getline(_packet.message.text,sizeof(_packet.message.text));
				write(sockfd, &_packet, sizeof(packet));								
				
				if(!strcmp(_packet.message.text,"close"))
					break;
					
				cout<<"Want to send another message(m) or wait for reply(r) or 	CLOSE"<<endl;
				cin>>ans;
				if(ans[0]=='r')
				{
					temp = read(sockfd, &_packet, sizeof(_packet));
					swap(_packet.message.dest_port, _packet.message.ret_port);
					cout<<"Message: "<<_packet.message.text<<endl;					
				}
				if(temp = read(sockfd, &_packet, sizeof(_packet)) > 0)
				{
					cout<<"You have a new message: "<<_packet.message.text<<endl;
					swap(_packet.message.dest_port, _packet.message.ret_port);
				}
				if(ans=="close")
				{
					strcpy(_packet.message.text, ans.c_str());
					write(sockfd, &_packet, sizeof(packet));
				}			
			}
		}
		else
		{
			if(temp = read(sockfd, &_packet, sizeof(_packet)) > 0)
			{
				cout<<"Message: "<<_packet.message.text<<endl;
				cout<<"Send a reply or enter close to end connection"<<endl;
				while(strcmp(_packet.message.text,"close"))
				{
					cin.ignore();
					cin.getline(_packet.message.text,sizeof(_packet.message.text));
					swap(_packet.message.dest_port, _packet.message.ret_port);
					write(sockfd, &_packet, sizeof(packet));
					
					if(!strcmp(_packet.message.text,"close"))						
						break;
								
					cout<<endl<<endl;
					cout<<"Want to send another message or wait for reply(r) or CLOSE"<<endl;
					cin>>ans;
					if(ans[0]=='r')
					{
						temp = read(sockfd, &_packet, sizeof(_packet));
						cout<<"Message: "<<_packet.message.text<<endl;					
					}
					if(ans=="close")
					{
						strcpy(_packet.message.text, ans.c_str());
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
		bzero(_packet.message.text,sizeof(_packet.message.text));
	}
	close(sockfd);
	} 
} 
