//server with client functionality 
#include "server.h"

class hybrid_server : public server
{
    int server_socket;  //socket to which this server will connect
    struct sockaddr_in server_addr;

    
    void set_rset()
    {
        server::set_rset();
        FD_SET(server_socket, &rset);   //set server socket
    }

    void accept_connection()
    {
        server::accept_connection();
        //set up a packet with type "connection" and initialize routing_tuple to broadcast new connection
        packet _packet(connection);
        _packet.new_tuple = routing_tuple(cliaddr.sin_port ,0 ,my_addr.sin_port, cliaddr.sin_port);
        
        table.update_table(_packet.new_tuple);
        
        //send to server
        write(server_socket, &_packet, sizeof(_packet)); 

        

    }

    void handle_client_packet()
    {

        for(auto it = client_sockets.begin(); it != client_sockets.end(); ++it)
        {
            if(FD_ISSET(it->first, &rset))
            {
                //read message
                packet _packet;
                read(it->first, &_packet, sizeof(_packet));
                bool for_client = false;
                if (_packet.type == msg)
                {
                    cout<<"Message: "<<_packet.message.text<<endl;
                    for(auto ij = client_sockets.begin(); ij != client_sockets.end(); ++ij)
                    {
                        if(ij->second == _packet.message.dest_port)
                        {
                            cout<<"sent packet to client\n";
                            write(ij->first, &_packet, sizeof(_packet));
                            for_client = true;
                            break;
                        }
                    }

                    if(!for_client)
                    {
                        //send to server 3
                        cout<<"sent packet to server 3\n";
                        write(server_socket, &_packet, sizeof(_packet));

                    }
                } 
                else if(_packet.type == dns_request)
                {

                }
            }
            //checking if message for its own client
            
        }
       
    }

    void handle_server_packet()
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

            table.update_table(_packet.new_tuple);


        }
        if(_packet.type == msg)
        {
            for(auto it = client_sockets.begin(); it != client_sockets.end(); ++it)
            {
                if(it->second == _packet.message.dest_port)
                {
                    //send to client
                    write(it->first, &_packet, sizeof(_packet));
                    cout<<"Message sent to client"<<endl;
                }
            }
        }		
    }
    public:
    hybrid_server(int port, int concurrent_clients, int server_port) : server(port, concurrent_clients)
    {
        server_socket = socket(AF_INET, SOCK_STREAM, 0); 

	    //set up server address
        bzero(&server_addr, sizeof(server_addr)); 
        server_addr.sin_family = AF_INET; 
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
        server_addr.sin_port = htons(server_port);

        //connect to server 
        if (connect(server_socket, (struct sockaddr*)&server_addr, 
                                sizeof(server_addr)) < 0) { 
            cout<<"\n Error : Connect Failed \n"; 
        } 
    }

    void run_server()
    {
        for(;;)
        {
            maxfdp1 = max(listenfd, server_socket);

            set_rset();

            nready = select(maxfdp1+1, &rset, NULL, NULL, NULL); 

            if(FD_ISSET(listenfd, &rset))
            {
                accept_connection();
            }
            else if(FD_ISSET(server_socket, &rset))
            {
                handle_server_packet();
            }
            else
            {
                handle_client_packet();
            }
        }
    }
};