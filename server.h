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
#include "packet.h"
#include "routing_table.h"
#include <iostream>
using namespace std;
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
   
    //relays a message to another socket such that it gets one hop closer to its destination...may send directly to client
    void relay(map<int, int> &sockets, packet &_packet)
    {
        int next_port = table.get_next_port(_packet.message.dest_port);	
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
        
        client_sockets.emplace(connfd, cliaddr.sin_port);		 
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
                _packet.message.ret_port=it->second;
                //if this is a new "connection" packet, change data for self and update in the routing table
                //then send to all connected servers except the one that sent the packet
                if(_packet.type == connection)
                {
                    _packet.new_tuple.router_port = my_addr.sin_port;
                    _packet.new_tuple.next_port = it->second;
                    _packet.new_tuple.num_hops++;

                    table.update_table(_packet.new_tuple);

                    broadcast(broadcast_sockets, it->first, _packet);
                }
                else if(_packet.type == msg)
                    relay(relay_sockets, _packet);
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
            
            handle_packet(client_sockets, client_sockets, client_sockets);
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
