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
    int get_next_port(int port)
    {
    	for(auto it = table.begin(); it!= table.end(); ++it)
        {
            if(it->getdestination_port() == port)
            	return it->getNext_port();
        }
        return -1;
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
