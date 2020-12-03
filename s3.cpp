//server 3
#include "proxy_server.h"
#define MAXLINE 1024 

int main() 
{ 
	proxy_server me(proxy, 3, 3);
	me.run_server();
}
