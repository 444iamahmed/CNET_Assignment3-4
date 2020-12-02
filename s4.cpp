//server 4
#include "hybrid_server.h"
#define PORT 5004 
#define MAXLINE 1024 


int main() 
{ 
	vector<int> server_sockets = {5003, 5000};
	hybrid_server me(PORT, 10, server_sockets);
	me.run_server();
}
