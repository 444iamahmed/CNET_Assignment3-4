//dns server
#include "dns_server.h"
#define MAXLINE 1024 


int main() 
{ 
	dns_server me(dns, 3, "test_file.txt");
	me.run_server();
}
