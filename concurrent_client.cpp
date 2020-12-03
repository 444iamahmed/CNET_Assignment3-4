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
#include <pthread.h>
#define MAXLINE 1024
using namespace std;

//global variables due to threading 

int sockfd, n, connection_message;
struct sockaddr_in servaddr; 



pthread_t receive_thread, main_thread;
//thread to receive any message from server
//DO NOT TAKE INPUT HERE
void* Receive(void* arg )
{
    packet _packet;
    
    do
    {
        read(sockfd,&_packet, sizeof(packet)); // read from server and output contents
        //display studf
    }while(true);    //recv again if the received message was not of type 0 meaning it was not a response of sent query
    
    pthread_exit(0);

}

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
    
    
    //start receive thread
    
    else
	{
        pthread_create(&receive_thread, NULL, Receive, NULL);
		bzero(&my_addr, sizeof(my_addr));
		len = sizeof(my_addr);
		getsockname(sockfd, (struct sockaddr *) &my_addr, &len);

		int readflags = fcntl(sockfd, F_GETFL, 0), temp;
		readflags = fcntl(sockfd, F_SETFL, readflags | O_NONBLOCK);
		

		for(;;)
		{
        //take input and write and shid
        }
    }

    pthread_join(receive_thread, NULL);
    close(sockfd); 
    pthread_exit(0);
} 