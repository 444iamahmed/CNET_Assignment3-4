#pragma once
#include "server.h"
#include <fstream>
class dns_server : public server
{
    protected:
    unordered_map<string, string> ips;  //ds w all ips
    ifstream ip_file;   //file with all ips 
    void update_recent_requests(dns_request &_dns_request){};   //no implementation needed
    void accept_connection()
    {
        server::accept_connection();
        server_connection_setup();
    }

    void process_request(packet &_packet)
    {
        string temp_req(_packet._dns_request.request);
        auto t = ips.find(temp_req);
        bzero(_packet._dns_request.response, 1000);
        if(t != ips.end())  //if response found
        {
            strcpy(_packet._dns_request.response,(t->second +"\nFOUND ON DNS\n").c_str());
            _packet._dns_request.found_at_dns = true;   //set true for update on proxy server

        }

        else
        {
            strcpy(_packet._dns_request.response, "NOT FOUND ON PROXY OR DNS\n");
        }

        _packet.dest_port = _packet.source_port;    //send back to client
        relay(client_sockets, _packet);
    }

    public:
    dns_server(int port, int concurrent_clients, string file_name) : server(port, concurrent_clients)
    {
        //read all responses from file
        ip_file.open(file_name);
        while(!ip_file.eof())
        {
            string request, response;
            ip_file>>request;
            ip_file>>response;
            ips.insert({request, response});
        }
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
};
