// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include  <time.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include "env_scpiparser.h"
#define PORT 9090
int main(int argc, char const *argv[]){

    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char resp[1024];

    scpi_setup();// initialize the parser

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt)))  {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    int lc=0;
         while ( 1)  {
            if (listen(server_fd, 3) < 0) {  perror("listen");   exit(EXIT_FAILURE);     }
			if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){
				perror("accept");               exit(EXIT_FAILURE);
			}

			valread = read( new_socket , buffer, 1024);
			buffer[valread]='\0';
			printf("This is from the client : %s length %d  expect %d ",buffer,strlen(buffer),valread );
			float hum, Temp;

			// remove the new line    
			//char * p = strchr(resp,'\n');	if ( p)  { *p = '\0' ;}

					if (valread > 0) {
								env_scpi_execute_command( buffer, valread);
								strcpy(resp,	env_get_result());
								if(strlen(resp)== 0) { strcpy(resp, "wrong SCPI cmd");}
					} else {
						strcpy(resp, "message is zerro");
					}
				printf( "will send %s\n\r ", resp);
			send(new_socket , resp , strlen(resp) , 0 );
			//printf("resp message sent\n");
        }
    return 0;
}
