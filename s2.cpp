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
#include<map>
#include "packet.h"
#include "routing_table.h"
#include<iostream>
#define PORT 5002
#define MAXLINE 1024 
using namespace std;

int max(int x, int y) 
{ 
	if (x > y) 
		return x; 
	else
		return y; 
} 
int main() 
{ 
	//server variables
	routing_table table;	//table to hold routing info
	map<int, int> client_sockets;	//map with all client sockets and corresponding ports such that each pair is: [socketfd, port]
	int listenfd, connfd, nready, maxfdp1, server_socket;  //listenfd = master socket, server_socket = server3 socket
	fd_set rset; 
	socklen_t len; 
	struct sockaddr_in server_addr, my_addr, cliaddr; 

    
	/* create listening TCP socket */
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	
	//set up own address
	bzero(&my_addr, sizeof(my_addr)); 
	my_addr.sin_family = AF_INET; 
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	my_addr.sin_port = htons(PORT); 

	//Create connect TCP socket
	server_socket = socket(AF_INET, SOCK_STREAM, 0); 

	//set up server address
	bzero(&server_addr, sizeof(server_addr)); 
	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	server_addr.sin_port = htons(5000); 

	// binding server addr structure to listenfd 
	bind(listenfd, (struct sockaddr*)&my_addr, sizeof(my_addr)); 
	listen(listenfd, 10); 

	//connect to server 3
	if (connect(server_socket, (struct sockaddr*)&server_addr, 
							sizeof(server_addr)) < 0) { 
		printf("\n Error : Connect Failed \n"); 
	} 
	
	// get maxfd 
	maxfdp1 = max(listenfd, server_socket)+1; 
	for (;;) { 

		FD_ZERO(&rset); 

		// set listenfd and server_socket
		FD_SET(listenfd, &rset); 
		FD_SET(server_socket, &rset);
		
		//iterate over client_sockets to set whichever is connected-------it->first = socketfd, it->second = port
		for(auto it = client_sockets.begin(); it != client_sockets.end(); ++it)
		{
			if(it->first > 0)
				FD_SET(it->first,&rset);
			if(it->first > maxfdp1)
				maxfdp1 = it->first + 1;

		}
		
		
		// select the ready descriptor 
		nready = select(maxfdp1, &rset, NULL, NULL, NULL); 

		// if tcp socket is readable then handle 
		// it by accepting the connection 
		if (FD_ISSET(listenfd, &rset)) 
		{ 
			
			len = sizeof(cliaddr); 
			connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len); 
		
			printf("Connected to client: %d\n", cliaddr.sin_port); 
			
			//set up a packet with type "connection" and initialize routing_tuple to broadcast new connection
			packet _packet(connection);
			_packet.new_tuple = routing_tuple(cliaddr.sin_port ,0 ,my_addr.sin_port, cliaddr.sin_port);
			
			//insert in own table
			table.insert(_packet.new_tuple);

			//display changes
			cout<<"UPDATED TABLE: \n";
			table.display();
			
			//send to server 3
			write(server_socket, &_packet, sizeof(_packet)); 

			//save fd for future in map
			client_sockets.emplace(connfd, cliaddr.sin_port);		 
		}
		//if data rec from server
		else if(FD_ISSET(server_socket, &rset))
		{
			//set up packet for further comm and read
			packet _packet;
			read(server_socket, &_packet, sizeof(_packet));
			
			//if it is a broadcasted new "connection" packet, set it up and insert in own table
			if(_packet.type == connection)
			{
				_packet.new_tuple.router_port = my_addr.sin_port;
				_packet.new_tuple.next_port = server_addr.sin_port;
				_packet.new_tuple.num_hops++;

				table.insert(_packet.new_tuple);
				cout<<"UPDATED TABLE: \n";
				table.display();
			}	
		}
		//if a client sent data
		else
		{
			for(auto it = client_sockets.begin(); it != client_sockets.end(); ++it)
			{
				packet _packet;
				if(FD_ISSET(it->first, &rset))
				{
					//if packet is "msg"
					//read message
					cout<<"Message received by server 3"<<endl; 
					read(it->first, &_packet, sizeof(_packet));
					//send to client
					if(it->second == _packet.message.dest_port)
					{
						cout<<"sending to client"<<endl;
						write(it->first, &_packet, sizeof(_packet));
					}
					//if packet is "dns_request"
				}
			}
		}
	} 
}
