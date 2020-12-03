//server with client functionality 
#pragma once
#include "server.h"

class hybrid_server : public server
{
    protected:
    map<int, int> server_sockets, all_sockets;  //sockets to which this server will connect and "all_sockets" including clients and servers both
    struct sockaddr_in server_addr;

    void process_request(packet &_packet){};    
    void update_recent_requests(dns_request &_dns_request){};
    void set_rset()
    {
        server::set_rset();
        for(auto it = server_sockets.begin(); it != server_sockets.end(); ++it)
            FD_SET(it->first, &rset);   //set server sockets
    }

    
    void accept_connection()
    {
        server::accept_connection();

        client_sockets.emplace(connfd, cliaddr.sin_port);   //add client to client_sockets
        all_sockets.emplace(connfd, cliaddr.sin_port);  //add client to all_sockets as well
        //set up a packet with type "connection" and initialize routing_tuple to broadcast new connection
        
        packet _packet(routing_tuple(cliaddr.sin_port ,0 ,ntohs(my_addr.sin_port), cliaddr.sin_port));
        
        table.update_table(_packet.new_tuple);
        
        //send new connection to servers
        for(auto it = server_sockets.begin(); it != server_sockets.end(); ++it)
            write(it->first, &_packet, sizeof(_packet)); 
        

    }

    
    public:
    //gets all the ports for the servers this one will connect to ("server_ports") and connects
    hybrid_server(int port, int concurrent_clients, vector<int> server_ports) : server(port, concurrent_clients)
    {
        
        for(auto it = server_ports.begin(); it != server_ports.end(); ++it)
        {
            pair<map<int,int>::iterator, bool> temp = server_sockets.emplace(socket(AF_INET, SOCK_STREAM, 0), (*it)); //place in server_socket map and get pair
            all_sockets.emplace(*(temp.first)); //place in all_sockets as well
            
            //set up each server address
            bzero(&server_addr, sizeof(server_addr)); 
            server_addr.sin_family = AF_INET; 
            server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
            server_addr.sin_port = htons(*it);

            //connect to each server if it was placed in the map
           if(temp.second)
            {
                if (connect((temp.first)->first, (struct sockaddr*)&server_addr, 
                                    sizeof(server_addr)) < 0) { 
                cout<<"\n Error : Connect Failed \n"; 
                }
                
                //send own port to new server for server to server routing
                write(temp.first->first, &port, sizeof(int));
                
                //set up new connection
                routing_tuple temp_tuple(*it,0,ntohs(my_addr.sin_port),*it);    
                packet temp_packet(temp_tuple);
                temp_packet.source_port = ntohs(my_addr.sin_port);
            
                //send connection to all connected servers except the new one
                for(auto ij = server_sockets.begin(); ij != server_sockets.end(); ++ij)
                    if(ij->first != temp.first->first)
                    {
                        write(ij->first, &temp_packet, sizeof(packet)); 
                    }

                table.update_table(temp_tuple); //update table for self

            } 
        }
    }

    void run_server()
    {
        maxfd = max(listenfd, server_sockets.rbegin()->first);

        for(;;)
        {

            set_rset();

            nready = select(maxfd+1, &rset, NULL, NULL, NULL); 

            if(FD_ISSET(listenfd, &rset))
                accept_connection();

            handle_packet(all_sockets, server_sockets, all_sockets);    //check all connected sockets; broadcast only to server_sockets; relay is possible to any socket
            
        }
    }


};