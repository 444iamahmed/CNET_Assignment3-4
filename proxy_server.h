#pragma once
#include "server.h"

class proxy_server : public server
{
    protected:
    unordered_map<string, string> recent_requests;
    int max_requests;   //max recent requests stored on proxy server
    
    //erases substring from a string
    void eraseSubStr(std::string & mainStr, const std::string & toErase)
    {
        // Search for the substring in string
        size_t pos = mainStr.find(toErase);
        if (pos != std::string::npos)
        {
            // If found then erase it from string
            mainStr.erase(pos, toErase.length());
        }
    }

    
    void accept_connection()
    {
        server::accept_connection();
        server_connection_setup();  //set up server to server routes
    }

    void update_recent_requests(dns_request &_dns_request)
    {
        //insert new request in map
        string temp_req(_dns_request.request), temp_resp(_dns_request.response);
        eraseSubStr(temp_resp, "\nFOUND ON DNS");
        cout<<"updating... success from dns for: "<<temp_req<<" = "<<temp_resp<<"\n";
        recent_requests.insert({temp_req, temp_resp});
        if(recent_requests.size() > max_requests)   //delete last one if there are more requests than the specified max
        {
            cout<<"no error here\n";
            auto it = recent_requests.begin(), last = it;
            for(; it != recent_requests.end(); ++it)
                last = it;
            recent_requests.erase(last);
        }
    }
    //looks for response on proxy server
    void process_request(packet &_packet)   
    {
        
        auto t = recent_requests.find(_packet._dns_request.request);
        if(t != recent_requests.end())  //if found, set up data and set destination back to source
        {
            _packet.dest_port = _packet.source_port;
            bzero(_packet._dns_request.response, 1000);
            strcpy(_packet._dns_request.response, (t->second + "FOUND ON PROXY\n").c_str());
        }
        else
        {
            _packet.dest_port = dns;    //else set destination to dns to look there
        }
        relay(client_sockets, _packet); //send to set destination
    }
    public:
    proxy_server(int port, int concurrent_clients, int _max_requests) : server(port, concurrent_clients)
    {
        max_requests = _max_requests;
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
