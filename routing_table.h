#pragma once
#include<iostream>
#include <vector>
#include "routing_tuple.h"
using namespace std;
class routing_table //simple vector container with specialised ftns
{
	
    vector<routing_tuple> table;

    public:
    void display()
    {
        cout<<"DEST_PORT\tNUM_HOPS\tROUTER_PORT\tNEXT_PORT\n";
        for(auto it = table.begin(); it!= table.end(); ++it)
        {
            cout<<*it<<"\n";
        }
    }
    int get_next_port(int port) //gets next port for passed destination
    {
    	for(auto it = table.begin(); it!= table.end(); ++it)
        {
            if(it->destination_port == port)
            	return it->next_port;
        }
        return -1;
    }

    //checks whether the link to be used is in use by other client(s)
    bool link_in_use(int my_port, int source_port, int dest_port)   
    {
        if(dest_port == my_port)    //if destination is this server, link is not in use
            return false;
        int temp_next_port = get_next_port(dest_port);
        for(auto it = table.begin(); it!= table.end(); ++it)
		{
		    if(it->destination_port == temp_next_port && it->router_port == my_port)    //find tuple directly connected to this router with destination equal to next port
		    {
                if(it->clients_on_link[0] != -1 && it->clients_on_link[1] != -1)    //check if this tuple's link is set
                {
                    if((it->clients_on_link[0] == source_port && it->clients_on_link[1] == dest_port) || (it->clients_on_link[0] == dest_port && it->clients_on_link[1] == source_port))
                        return false;   //if it is, only ret false in case the requesting hosts are already using the link
                }
                else
                    return false;    //if not set, the link is free            
            }
        }
        return true;    //isnt free in any other case
    }

    //sets links corresponding to source and destination
    void set_links(int my_port, int source_port, int dest_port)
    {

        int temp_next_port_source = get_next_port(source_port); // wherever the pkt came from

        int temp_next_port_dest;
        
        if(my_port == dest_port) 
            temp_next_port_dest = my_port;  //dont set if the destination is this router
        else
            temp_next_port_dest = get_next_port(dest_port); //wherever the pkt will be sent next

        for(auto it = table.begin(); it!= table.end(); ++it)
		    if(it->router_port == my_port && (it->destination_port == temp_next_port_source || it->destination_port == temp_next_port_dest))    //set all links directly connected to router that are on the route
            {
                it->clients_on_link[0] = source_port;
                it->clients_on_link[1] = dest_port;
            }
        
    }

    //opposite of set_links
    void reset_links(int my_port, int source_port, int dest_port)
    {
        int temp_next_port_source = get_next_port(source_port);
        int temp_next_port_dest;
        
        if(my_port == dest_port) 
            temp_next_port_dest = my_port;
        else
            temp_next_port_dest = get_next_port(dest_port);
        
        for(auto it = table.begin(); it!= table.end(); ++it)
		    if(it->router_port == my_port && (it->destination_port == temp_next_port_source || it->destination_port == temp_next_port_dest))
            {
                it->clients_on_link[0] = -1;
                it->clients_on_link[1] = -1;
            }
    }

    //gets the links for a specific destination
    void get_clients_on_link(int my_port, int dest_port, int* clients_on_link)
    {
        int temp_next_port_dest = get_next_port(dest_port);

        for(auto it = table.begin(); it!= table.end(); ++it)
		    if(it->router_port == my_port && it->destination_port == temp_next_port_dest)
            {
                clients_on_link[0] = it->clients_on_link[0];
                clients_on_link[1] = it->clients_on_link[1];
            }
    }
	
    void update_table(routing_tuple new_tuple)
    {
        table.push_back(new_tuple);
        cout<<"UPDATED TABLE: \n";
        display();
    }

    vector<routing_tuple>::iterator begin()
    {
        return table.begin();
    }
    vector<routing_tuple>::iterator end()
    {
        return table.end();
    }
};
