#pragma once
#include "routing_tuple.h"
#include "dns_request.h"
//data in this system is transmitted in the form of "packets" with diff types
//"connction" packets are broadcasted for new connections
//"msg" packets are messages b/w clients
//"dns_request" packets are requests for ips from the dns server
enum Type {connection, msg, dns_req, proxy_update};   
enum destPort {proxy = 5003, dns = 5000};

struct packet
{
    int type;
    routing_tuple new_tuple;    //for connection
    char message[1000];    //for msg
    dns_request _dns_request;   //for dns request
    int source_port;
    int dest_port;

    
    packet()
    {
    }
    packet(Type _type)
    {
        type = _type;
    }
    packet(routing_tuple _new_tuple)
    {
        type = connection;
        new_tuple = _new_tuple;
    }
    packet(dns_request update)   //only use for proxy update generation
    {
        type = proxy_update;
        strcpy(_dns_request.request, update.request);
        strcpy(_dns_request.response, update.response);
        _dns_request.found_at_dns = update.found_at_dns;
        _dns_request.num_ports_visited = update.num_ports_visited;
        for(int i = 0; i < _dns_request.num_ports_visited; ++i)
            _dns_request.ports_visited[i] = update.ports_visited[i];
        dest_port = proxy;
    }
    packet(string DNS_request, int s_port)  //only for new dns requests
    {
        type = dns_req;
        strcpy(_dns_request.request,DNS_request.c_str());
        dest_port = proxy;
        source_port = s_port;
    }
    packet(string _m, int d_port, int s_port)
    {
        strcpy(message, _m.c_str());
        dest_port = d_port;
        source_port = s_port;
        type = msg;
    }
   
};