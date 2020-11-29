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
#define PORT 5000 
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
	map<int, int> client_sockets;	
	routing_table table;
	int listenfd, connfd, nready, maxfdp1;  
	fd_set rset; 
	socklen_t len; 
	struct sockaddr_in cliaddr, my_addr; 


	
	/* create listening TCP socket */
	listenfd = socket(AF_INET, SOCK_STREAM, 0); 
	bzero(&my_addr, sizeof(my_addr)); 
	my_addr.sin_family = AF_INET; 
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	my_addr.sin_port = htons(PORT); 

	// binding server addr structure to listenfd 
	bind(listenfd, (struct sockaddr*)&my_addr, sizeof(my_addr)); 
	listen(listenfd, 3); 

	
	// get maxfd 
	maxfdp1 = listenfd+1; 
	for (;;) { 

		FD_ZERO(&rset); 

		// set listenfd and udpfd in readset 
		FD_SET(listenfd, &rset); 

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
		
			printf("Connected to server: %d\n", ntohs(cliaddr.sin_port)); 
			
			client_sockets.emplace(connfd, ntohs(cliaddr.sin_port));		 
		}
		//if data rec from another server
		else
		{
			for(auto it = client_sockets.begin(); it != client_sockets.end(); ++it)
			{
				if(FD_ISSET(it->first, &rset))
				{
					//set up and read packet
					packet _packet;
					read(it->first, &_packet, sizeof(_packet));
					//if this is a new "connection" packet, change data for self and update in the routing table
					//then send to all connected servers except the one that sent the packet
					if(_packet.type == connection)
					{
						_packet.new_tuple.router_port = my_addr.sin_port;
						_packet.new_tuple.next_port = it->second;
						_packet.new_tuple.num_hops++;

						table.insert(_packet.new_tuple);
						cout<<"UPDATED TABLE: \n";
						table.display();

						for(auto ij = client_sockets.begin(); ij != client_sockets.end(); ++ij)
						{
							if(ij->second != it->second)
							write(ij->first, &_packet, sizeof(_packet));
						}
					}
				}
			}
		}
	} 
}
