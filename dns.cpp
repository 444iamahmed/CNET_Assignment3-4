//dns server
#include "server.h"
#define PORT 5000
#define MAXLINE 1024 


int main() 
{ 
	server me(PORT, 3);
	me.run_server();
}
