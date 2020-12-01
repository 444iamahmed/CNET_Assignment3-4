//server 2
#include "hybrid_server.h"
#define PORT 5002 
#define MAXLINE 1024 

int main() 
{ 
	hybrid_server me(PORT, 10, 5000);
	me.run_server();
}
