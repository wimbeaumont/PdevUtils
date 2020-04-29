//============================================================================
// Name        : scpiclient.cpp
// Author      : Wim
// Version     :
// Copyright   : Willem Beaumont University Antwerpen
// Description : program to test a SCPI server based on sockets
//============================================================================

// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#define PORT 9090

int main(int argc, char const *argv[]){
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[255];
    //char *hello = "Hello from client";
    char hello[1024];

    const int NrCmd=6;
    char line_buffer[NrCmd][256];



    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

	int lc=0;

			strcpy(line_buffer[lc++],"*IDN?");
			strcpy(line_buffer[lc++],":MEAS:TEMP?:CH 1");
			strcpy(line_buffer[lc++],":MEAS:TEMP?");
			strcpy(line_buffer[lc++],":ONZIN:TEMP?");
			strcpy(line_buffer[lc++],":MEASURE:HUMI?");
			strcpy(line_buffer[lc++],":MEASURE:LUMINOSITY?");
	 int nrmsg=lc;
	 if (nrmsg > NrCmd) {
		 printf("more mesages defined than fits in array \n\r");
	 	 nrmsg=NrCmd;
	 }
 	 lc=0;

         while(lc< nrmsg) {
                 if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)       {
             printf("\n Socket creation error \n");
             return -1;
         }
         if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
         {
             printf("\nConnection Failed \n");
             return -1;
         }
         send(sock , line_buffer[lc] , strlen(line_buffer[lc]) , 0 );
         printf("sent msg nr %d  %s length %d \n",lc,line_buffer[lc],strlen(line_buffer[lc]) );
         buffer[0]='\0';
         valread = read( sock , buffer, 1024);
         buffer[valread]='\0';
         printf("this is the response (length %d)  from the server : %s %d\n",valread,buffer, strlen(buffer) );
                 close(sock);
                 sleep(1);
          lc++;

    }

  return 0;
}
