#pragma once
#include<string>
#include<vector>
using namespace std;
struct dns_request
{
    char request[1000];  //for dns_req
    char response[1000]; //for dns_req
    int ports_visited[20];
    int num_ports_visited=0;
    bool found_at_dns = false;
};