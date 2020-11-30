#pragma once
#include "routing_tuple.h"
#include "message.h"
//data in this system is transmitted in the form of "packets" with diff types
//"connction" packets are broadcasted for new connections
//"msg" packets are messages b/w clients
//"dns_request" packets are requests for ips from the dns server
enum Type {connection, msg, dns_request};   

struct packet
{
    Type type;
    routing_tuple new_tuple;    //for connection
    Message message;    //for msg
    char request[256];  //for dns_req
    char response[256]; //for dns_req
    packet(Type _type)
    {
        type = _type;
    }
    packet()
    {
        type = connection;
    }
};