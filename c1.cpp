#include <netinet/in.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <limits>
#include <fcntl.h>
#include<iostream>
#include <semaphore.h>
#include "packet.h"
#include <pthread.h>
#define MAXLINE 1024
using namespace std;

//global variables due to threading 

bool new_comm;
int sockfd, clientno, choice1 ,choice2, dest_port, connected_port = -1; 
struct sockaddr_in my_addr, servaddr; 
sem_t semsem; 


pthread_t receive_thread;
//thread to receive any message from server
//DO NOT TAKE INPUT HERE
void* Receive(void* arg )
{
    packet _packet;
    
    do
    {
        read(sockfd,&_packet, sizeof(packet)); // read from server and output contents

        if(_packet.type == dns_req) //dns req results
        {
            sem_wait(&semsem);
            //display route
            cout<<_packet._dns_request.response<<"FOR "<<_packet._dns_request.request<<"\nROUTE: "<<my_addr.sin_port<<" -> ";
            for(int i = 0; i < _packet._dns_request.num_ports_visited; ++i)
            {
                cout<<_packet._dns_request.ports_visited[i]<<" -> ";
                if(i+1 >= _packet._dns_request.num_ports_visited)
                    cout<<my_addr.sin_port<<"\n";
            }
            cin.clear();
            cout<<"INPUT FOR CURRENT PROMPT AGAIN: \n";
            sem_post(&semsem);
        }
        else if(_packet.type == msg)    //msg results
        {
            sem_wait(&semsem);
            cout<<_packet.source_port<<": "<< _packet.message<<"\n";    //display rec message
            if(string(_packet.message).find("LINK BUSY") != string::npos || !strcmp(_packet.message,"close"))
                connected_port = -1;    //if link busy or connection ending, allow for new connection
            if(_packet.new_comm)
                connected_port = _packet.source_port;   //allow for continued two sided comm if this is a new communication
            cin.clear();
            cout<<"INPUT FOR CURRENT PROMPT AGAIN: \n";
            sem_post(&semsem);
        }
    }while(true);
    
    pthread_exit(0);

}

int main() 
{ 
    bool check;
    fd_set rset;
	string request, message, ans;
	//char buffer[MAXLINE], ans; 
	//char* message; 
	int n;
	socklen_t len; 
    int semvalue_catch;
    sem_init(&semsem, 0, 1);

	// Creating socket file descriptor 
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
		cout<<"socket creation failed"; 
		exit(0); 
	} 

	memset(&servaddr, 0, sizeof(servaddr)); 

	// Filling server information 
	servaddr.sin_family = AF_INET;
	
    do
    {
        cout<<"The Servers are: \n 5001 \n 5002 \n 5004 \n"; 
    	cout<<"ENTER SERVER TO CONNECT TO: \n";
	    cin>>servaddr.sin_port;
    }while(servaddr.sin_port != 5001 && servaddr.sin_port != 5002 && servaddr.sin_port != 5004);    //can only connect to these servers
	
	servaddr.sin_port=htons(servaddr.sin_port);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

	if (connect(sockfd, (struct sockaddr*)&servaddr, 
							sizeof(servaddr)) < 0) { 
		cout<<"\n Error : Connect Failed \n"; 
	} 
    
    
    
    else
	{
        pthread_create(&receive_thread, NULL, Receive, NULL);    //start receive thread
		bzero(&my_addr, sizeof(my_addr));
		len = sizeof(my_addr);
		getsockname(sockfd, (struct sockaddr *) &my_addr, &len);    //set up own address 


		for(;;)
		{
            sem_getvalue(&semsem, &semvalue_catch);
            if(semvalue_catch)
            {
                cout<<"Enter \n 1: To send DNS request \n anything else: To connect to client \n";
                cin>>choice1;
                cin.clear();
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            if(choice1 == 1)
            {
                sem_getvalue(&semsem, &semvalue_catch);
                if(semvalue_catch)
                {
                    cout<<"ENTER DNS REQUEST: \n";
                    cin>>request;
                    cin.clear();
                    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
                packet _packet(request, my_addr.sin_port);
                write(sockfd, &_packet, sizeof(packet));
            
            }
            else
            {
                sem_getvalue(&semsem, &semvalue_catch);
                if(semvalue_catch)
                {
                    cout<<"Enter client port to connect to: ";
                    cin>>dest_port;
                    cin.clear();
                    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
                sem_wait(&semsem);
                new_comm = dest_port != connected_port;
                connected_port = dest_port;
                sem_post(&semsem);
                do
                {
                    sem_getvalue(&semsem, &semvalue_catch);
                    if(semvalue_catch)
                    {
                        cout<<"Enter message (first msg in a new connection can't be \"close\"): "<<endl;
                        getline(cin, message);  
                    }
                    
                }while(new_comm && message=="close");
                
                packet _packet(message, dest_port, my_addr.sin_port, new_comm); 
                write(sockfd, &_packet, sizeof(_packet));
            }
            
        //take input and write and shid
        }
    }

    pthread_join(receive_thread, NULL);
    close(sockfd); 
    pthread_exit(0);
} 