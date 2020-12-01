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
using namespace std;
class server
{
    protected:
    //server variables
	routing_table table;	//table to hold routing info
	map<int, int> client_sockets;	//map with all client sockets and corresponding ports such that each pair is: [socketfd, port]
	int listenfd, connfd, nready, maxfdp1, server_socket;  //listenfd = master socket   
    fd_set rset; 
	socklen_t len; 
	struct sockaddr_in cliaddr, my_addr; 
   

    void set_rset()
    {
        FD_ZERO(&rset); 

		// set listenfd 
		FD_SET(listenfd, &rset); 		

		//iterate over client_sockets to set whichever is connected-------it->first = socketfd, it->second = port
		for(auto it = client_sockets.begin(); it != client_sockets.end(); ++it)
		{
			if(it->first > 0)
				FD_SET(it->first,&rset);
			if(it->first > maxfdp1)
				maxfdp1 = it->first;

		}

    
    }
    void accept_connection()
    {
        len = sizeof(cliaddr); 
        connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len); 
        
        cout<<"Connected to: "<<cliaddr.sin_port<<"\n";
        
        client_sockets.emplace(connfd, cliaddr.sin_port);		 
    }

    void handle_client_packet()
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

                    table.update_table(_packet.new_tuple);

                    for(auto ij = client_sockets.begin(); ij != client_sockets.end(); ++ij)
                    {
                        if(ij->second != it->second)
                        write(ij->first, &_packet, sizeof(_packet));
                    }
                }
                else if(_packet.type == msg)
                {	
                    int next_port = table.getnext_port(_packet.message.dest_port);
                    cout<<"rec msg "<<_packet.message.text<<" next port: "<<next_port<<"\n";	
                    for(auto ij = client_sockets.begin(); ij != client_sockets.end(); ++ij)
                    {
                        if(ij->second == next_port)
                        {
                            cout<<write(ij->first, &_packet, sizeof(_packet))<<" sent packet to next server "<<ij->first<<"\n";
                            
                        }
                    }													
    
                }
                else if(_packet.type == dns_request)
                {}
            }
        }
    }
    public:
    server(int port, int concurrent_clients)
    {
        listenfd = socket(AF_INET, SOCK_STREAM, 0); 
        bzero(&my_addr, sizeof(my_addr)); 
        my_addr.sin_family = AF_INET; 
        my_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
        my_addr.sin_port = htons(port); 

        // binding my_addr structure to listenfd 
        bind(listenfd, (struct sockaddr*)&my_addr, sizeof(my_addr)); 
        listen(listenfd, concurrent_clients); 
        
    }

    void run_server()
    {
        maxfdp1 = listenfd;
        for(;;)
        {
            set_rset();

            nready = select(maxfdp1+1, &rset, NULL, NULL, NULL); 

            if(FD_ISSET(listenfd, &rset))
            {
                accept_connection();
            }
            else
            {
                handle_client_packet();
            }
        }
    }
    
    int max(int x, int y) 
    { 
        if (x > y) 
            return x; 
        else
            return y; 
    } 
};