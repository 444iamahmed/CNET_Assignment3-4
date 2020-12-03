#pragma once
#include <string.h>
#include <arpa/inet.h> 
#include <errno.h> 
#include <netinet/in.h> 
#include <signal.h> 
#include <stdlib.h> 
#include <strings.h> 
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <fcntl.h>
#include <map>
#include <unordered_map>
#include "packet.h"
#include "routing_table.h"
#include <iostream>
using namespace std;
//abstract base server class
class server
{
    protected:
    //server variables
	routing_table table;	//table to hold routing info
	map<int, int> client_sockets;	//map with all client sockets and corresponding ports such that each pair is: [socketfd, port]
	int listenfd, connfd, nready, maxfd;  //listenfd = master socket   
    fd_set rset; 
	socklen_t len; 
	struct sockaddr_in cliaddr, my_addr; 
	
   
    virtual void process_request(packet &_packet) = 0;
    virtual void update_recent_requests(dns_request &_dns_request) = 0;
    //sets up connection and routing table for base servers such as proxy_server and dns_server
    void server_connection_setup()
    {
        int temp_port;
        read(connfd, &temp_port, sizeof(int));  //get port of client after connection
        cout<<"read port!\n";
        packet temp_packet;

        
        for(auto it = table.begin(); it != table.end(); ++it)   //send all existing tuples to new client
        {
            temp_packet = packet(*it);
            write(connfd, &temp_packet, sizeof(packet));
        }

        routing_tuple temp_tuple(temp_port, 0, htons(my_addr.sin_port), temp_port);
        temp_packet = packet(temp_tuple);

        for(auto it = client_sockets.begin(); it != client_sockets.end(); ++it) //sent new tuple to all existing clients
            write(it->first, &temp_packet, sizeof(packet));

        table.update_table(temp_tuple); //add new tuple to own table
        client_sockets.emplace(connfd, temp_port);  //add new client to own sockets
    }   

    //relays a message to another socket such that it gets one hop closer to its destination...may send directly to client
    void relay(map<int, int> &sockets, packet &_packet)
    {
        int next_port = table.get_next_port(_packet.dest_port);
        for(auto ij = sockets.begin(); ij != sockets.end(); ++ij)
        {
            if(next_port != -1 && ij->second == next_port)
            {
                write(ij->first, &_packet, sizeof(_packet));
                break;
            }    
                
        }
    }	
    
    //sends packet to all sockets except the one equal to parameter
    void broadcast(map<int,int> &sockets, int socket, packet &_packet)
    {
        for(auto ij = sockets.begin(); ij != sockets.end(); ++ij)
        {
            if(ij->first != socket)
                write(ij->first, &_packet, sizeof(_packet));
        }
    }

    //reset read_set for next iteration
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
			if(it->first > maxfd)
				maxfd = it->first;
		}
    }

    //accept a new connection from a client
    void accept_connection()
    {
        len = sizeof(cliaddr); 
        connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len); 
        
        cout<<"Connected to: "<<cliaddr.sin_port<<"\n";
        
    }

    //handles a packet
    //checks "checking_sockets" for activity
    //broadcasts packets to "broadcasting_sockets"
    //relays packets to "relay_sockets"
    void handle_packet(map<int, int> &checking_sockets, map<int, int> &broadcast_sockets, map<int, int> &relay_sockets)
    {
        for(auto it = checking_sockets.begin(); it != checking_sockets.end(); ++it)
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
                    cout<<"rec conn pkt from "<<_packet.source_port<<"\n";
                    _packet.new_tuple.next_port = it->second;
                    _packet.new_tuple.num_hops++;

                    table.update_table(_packet.new_tuple);

                    broadcast(broadcast_sockets, it->first, _packet);
                }

                //if this is a message, relay it closer to its destination
                else if(_packet.type == msg)
                {
                	if(_packet.link )
                	{
                		
                		
                		if(table.getting_client_no(_packet.dest_port) == _packet.dest_port )
                		{
                			cout<<"here"<<endl;
                			string mg= "LINK BUSY";
		        		strcpy(_packet.message, mg.c_str());
		        		swap(_packet.dest_port, _packet.source_port);
		        		write(it->first, &_packet, sizeof(_packet));
                		}
                		else
                		{cout<<"here1"<<endl;
                		int router_port=table.get_router_port(_packet.source_port);
                		table.setting_client_no(_packet.source_port, router_port);
                		
                		router_port=table.get_router_port(_packet.dest_port);
                		table.setting_client_no(_packet.dest_port, router_port);
                		
                		relay(relay_sockets, _packet);
                		}
                	}
                	else if (table.getting_client_no(_packet.source_port) == _packet.source_port || table.getting_client_no(_packet.dest_port) == _packet.dest_port || table.getting_client_no(_packet.source_port) == _packet.dest_port || table.getting_client_no(_packet.dest_port) == _packet.source_port )
                	{
                		cout<<"here2"<<endl;
                		relay(relay_sockets, _packet);
                	}
                	else
                	{
                		string mg= "LINK BUSY";
                		strcpy(_packet.message, mg.c_str());
                		swap(_packet.dest_port, _packet.source_port);
                		write(it->first, &_packet, sizeof(_packet));
                	}                	
             
                    
                }
                else if(_packet.type == dns_req)
                {
                    cout<<"rec dns pkt\n";
                    if(_packet._dns_request.found_at_dns)   //send an update to proxy server in the case of success from dns
                    {
                        _packet._dns_request.found_at_dns = false;  //just making sure the update is only sent once
                        packet update_packet(_packet._dns_request);
                        relay(relay_sockets, update_packet);
                    }
                    _packet._dns_request.ports_visited[_packet._dns_request.num_ports_visited++] = (ntohs(my_addr.sin_port));   //add current node to route

                    if(htons(_packet.dest_port) != my_addr.sin_port)    //if this server is not proxy or dns, relay
                        relay(relay_sockets, _packet);
                    else 
                        process_request(_packet);   
                }
                else if(_packet.type == proxy_update)
                {
                    update_recent_requests(_packet._dns_request);   //for proxy server so it can update the recent requests
                }
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

    //runs the server after setup
    void run_server()
    {
        maxfd = listenfd; //no other fd here, so max is listenfd
        for(;;)
        {
            set_rset(); //reset read_set

            nready = select(maxfd+1, &rset, NULL, NULL, NULL); 

            if(FD_ISSET(listenfd, &rset))
                accept_connection();
            
            handle_packet(client_sockets, client_sockets, client_sockets);  //base server only has client sockets so it checks, broadcasts to, and relays to just them
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
