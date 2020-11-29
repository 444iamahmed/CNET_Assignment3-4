#pragma once
#include <iostream>
using namespace std;
struct routing_tuple    //simple routing tuple to hold routing info
{
    int destination_port;
    int num_hops;
    int router_port;
    int next_port;

    routing_tuple(int _destination_port, int _num_hops, int _router_port, int _next_port)
    {
        destination_port = _destination_port;
        num_hops = _num_hops;
        router_port = _router_port;
        next_port = _next_port;
    }
    routing_tuple()
    {
        destination_port = 0;
        num_hops = 0;
        router_port = 0;
        next_port = 0;
    }
    //overloading cout
    friend ostream& operator<<(ostream& os, const routing_tuple& tuple);
};

ostream& operator<<(ostream& os, const routing_tuple& tuple)
{
    os << tuple.destination_port << "\t\t" << tuple.num_hops << "\t\t" << tuple.router_port << "\t\t" << tuple.next_port << "\n";
    return os;
}