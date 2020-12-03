#pragma once
struct Message  //text message to a designated port
{
    char text[1000];
    int dest_port;
    int ret_port;
};
