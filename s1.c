#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include<fcntl.h>  
     
#define TRUE   1  
#define FALSE  0  
#define PORT 8888
#define MAXLINE 1024  
     
int main(int argc , char *argv[])   
{   
	//client variables
	fd_set rset;
	int sockfd; 
	char buffer[MAXLINE]; 
	char* message; 
	struct sockaddr_in servaddr; 
	
	//server variables
	int opt = TRUE;   
    	int master_socket , addrlen , new_socket , client_socket[30] ,  max_clients = 30 , activity, i , valread , sd;   
    	int max_sd;   
    	struct sockaddr_in address;
    	
    	//set of socket descriptors  
    	fd_set readfds;
    
	int n, len; 
	// Creating socket file descriptor 
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
		printf("socket creation failed"); 
		exit(0); 
	} 

	memset(&servaddr, 0, sizeof(servaddr)); 
	
	//sockets for server side code//
	
	//initialise all client_socket[] to 0 so not checked  
   	 for (i = 0; i < max_clients; i++)   
    	{   
        client_socket[i] = 0;   
    	}   
         
    	//create a master socket  
    	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    	{   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    	}   
	//set master socket to allow multiple connections ,  
    	//this is just a good habit, it will work without this  
    	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )   
    	{   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
        
          
	// Filling server infFrom TCPormation for client
	servaddr.sin_family = AF_INET; 
	servaddr.sin_port = htons(8889); 
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	
	//type of socket created  for server side
    	address.sin_family = AF_INET;   
    	address.sin_addr.s_addr = INADDR_ANY;   
    	address.sin_port = htons( PORT );   
         
    	//bind the socket to localhost port 8888  
    	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)   
    	{   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    	}   
    printf("Listener on port %d \n", PORT);
    
      //try to specify maximum of 3 pending connections for the master socket  
    	if (listen(master_socket, 3) < 0)   
    	{   
        perror("listen");   
        exit(EXIT_FAILURE);   
    	}   
 
	//accept the incoming connection  
    	addrlen = sizeof(address);   
    	puts("Waiting for connections");      
	
	//connecting as client
	if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{ 
		printf("\n Error : Connect Failed \n"); 
	} 

	int readflags = fcntl(sockfd, F_GETFL, 0), temp;
	readflags = fcntl(sockfd, F_SETFL, readflags | O_NONBLOCK);
	while(temp = read(sockfd, buffer, sizeof(buffer)) < 0)
	{
	}
	printf("server: "); 
	puts(buffer);
	bzero(buffer,sizeof(buffer));
	
	//client + server same loop
	while(TRUE)   
    	{
	
	//////	ACCEPTING CONNECTIONS AS SERVER
	
	//clear the socket set  
        FD_ZERO(&readfds);   
     
        //add master socket to set  
        FD_SET(master_socket, &readfds);   
        max_sd = master_socket;   
             
        //add child sockets to set  
        for ( i = 0 ; i < max_clients ; i++)   
        {   
            //socket descriptor  
            sd = client_socket[i];   
                 
            //if valid socket descriptor then add to read list  
            if(sd > 0)   
                FD_SET( sd , &readfds);   
                 
            //highest file descriptor number, need it for the select function  
            if(sd > max_sd)   
                max_sd = sd;   
        }   
     
        //wait for an activity on one of the sockets , timeout is NULL ,  
        //so wait indefinitely  
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);   
       
        if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }   
             
        //If something happened on the master socket ,  
        //then its an incoming connection  
        if (FD_ISSET(master_socket, &readfds))   
        {   
            if ((new_socket = accept(master_socket,  
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)   
            {   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }   
             
            //inform user of socket number - used in send and receive commands  
            printf("New connection: \nsocket fd%d \nip: %s \nport: %d \n  " , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
            
            
            	 ////// ACTING AS CLIENT 
            	
		memset(buffer, 0, sizeof(buffer)); 

		if(temp = read(sockfd, buffer, sizeof(buffer)) > 0)
		{
			printf("server: "); 
			puts(buffer);
			bzero(buffer,sizeof(buffer));

		}
		printf("Message to server: ");
		scanf("%[^\n]%*c",buffer);
		write(sockfd, buffer, sizeof(buffer)); 
		 
	
         	///////TALKING WITH CLIENT
         	
	    	//a message  
	    	message = "Hi from server!! \r\n";            
           
            //send new connection greeting message  
            if( send(new_socket, message, strlen(message), 0) != strlen(message) )   
            {   
                perror("send");   
            }   
                 
            puts("Welcome message sent successfully");   
                 
            //add new socket to array of sockets  
            for (i = 0; i < max_clients; i++)   
            {   
                //if position is empty  
                if( client_socket[i] == 0 )   
                {   
                    client_socket[i] = new_socket;   
                    printf("Added to list of sockets as %d\n" , i);   
                         
                    break;   
                }   
            }   
        }   
             
        //else its some IO operation on some other socket 
        for (i = 0; i < max_clients; i++)   
        {   
            sd = client_socket[i];   
                 
            if (FD_ISSET( sd , &readfds))   
            {   
                //Check if it was for closing , and also read the  
                //incoming message  
                if ((valread = read( sd , buffer, 1024)) == 0)   
                {   
                    //Somebody disconnected , get his details and print  
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);   
                    printf("Host disconnected \n, ip %s \n, port %d \n" ,  inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   
                         
                    //Close the socket and mark as 0 in list for reuse  
                    close( sd );   
                    client_socket[i] = 0;   
                }   
                     
                //Echo back the message that came in  
                else 
                {   
                    //set the string terminating NULL byte on the end  
                    //of the data read  
                    buffer[valread] = '\0';   
                    send(sd , buffer , strlen(buffer) , 0 );   
                }   
            }   
        }   
    }   
         
    return 0;   
}
