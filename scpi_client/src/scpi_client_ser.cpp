/** 
 * Name        : scpiclientser.cpp
 * Author      : Wim
 * Version     :
 *  Description : program to test a SCPI device connected to a serial port 
 *  (C) Wim Beaumont Universiteit Antwerpen 2019 2020
 * License see
 *  https://github.com/wimbeaumont/PeripheralDevices/blob/master/LICENSE
 *  Version history :
 *  0.1   copied from scpiclient 
 *  1.2   added close ,  corrected message  '/0' pos
 
 */ 

#define SCPICLIENSERVER "1.4"


#include <cstdio>
//#include <errno.h>  
#include <fcntl.h>  // File Control Definitions for serial         
#include <termios.h>// POSIX Terminal Control Definitions for serial 
#include <cstdlib>     // atof 
#include <unistd.h> 		// for sockets , serial 
#include <string.h> 


int serialportsetup(char *portname) {
char SERPORTDEFAULT[]= "/dev/ttyACM0";	

if (! portname ) portname = SERPORTDEFAULT;
printf("\n  try to open %s\n",portname);
int fd=open(portname,O_RDWR | O_NOCTTY); //fd is locally defined but the open is static so also have to be closed

	if(fd ==-1)
     printf("\n  Error! in Opening %s\n",portname);
	else{ 
		printf("\n  %s Opened Successfully\n",portname);
		
		struct termios SerialPortSettings;
		tcgetattr(fd, &SerialPortSettings);//get current settings 
		cfsetispeed(&SerialPortSettings,B9600);//read speed 
		cfsetospeed(&SerialPortSettings,B9600);//write speed 
		SerialPortSettings.c_cflag &= ~CSIZE ; // set char size mask to 0 
		SerialPortSettings.c_cflag |=  CS8 ; // set char size to 8 
		SerialPortSettings.c_cflag &= ~PARENB;   // No Parity
		SerialPortSettings.c_cflag &= ~CSTOPB; //Stop bits = 1 
		SerialPortSettings.c_cflag &= ~CRTSCTS; // no RTS .CTS 
		SerialPortSettings.c_lflag &= ~(ECHO | ECHOE | ECHONL) ; // Disable echo
		SerialPortSettings.c_lflag |=   ICANON;// read  full lines 
		SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY); // no flow control
		//SerialPortSettings.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any specia not expected
		tcsetattr(fd,TCSANOW,&SerialPortSettings); // write the settings to the hardware without waiting ( TCSANOW)
		//SerialPortSettings.c_cc[VMIN]  = 0; // Read 10 characters   we wait for the newline 
		SerialPortSettings.c_cc[VTIME] = 0;  // Wait indefinitely for the moment   
		
		
	}
    return fd ;
}

//============================================================================

int main(int argc, char const *argv[]){
	int valread; 
    char buffer[256];
    const int NrCmd=20;
    char line_buffer[NrCmd][256];
    char* portname=0;
    if ( argc==2) {
		portname=(char*)argv[1];
	}
	printf("%s ver %s\n\r",argv[0], SCPICLIENSERVER );
//serial port 
	int fd  = serialportsetup(portname);
	if ( fd == -1 ) exit(-1); 
	int lc=0;

			strcpy(line_buffer[lc++],"*IDN?");
			strcpy(line_buffer[lc++],":MEAS:TEMP?:CH 1");
			strcpy(line_buffer[lc++],":MEAS:TEMP0?");
			strcpy(line_buffer[lc++],":MEAS:TEMP1?");
			strcpy(line_buffer[lc++],":MEAS:TEMP2?");
			strcpy(line_buffer[lc++],":MEAS:TEMP? CH1");
			strcpy(line_buffer[lc++],":MEAS:TEMP? CH2");
			strcpy(line_buffer[lc++],":MEAS:TEMP? 1");
			strcpy(line_buffer[lc++],":MEAS:TEMP? 2");
			strcpy(line_buffer[lc++],"*HWVer?");
			strcpy(line_buffer[lc++],":MEAS:TEMP?");
			strcpy(line_buffer[lc++],":ONZIN:TEMP?");
			strcpy(line_buffer[lc++],":MEASURE:HUMI?");
			strcpy(line_buffer[lc++],":MEASURE:LUMINOSITY?");
			strcpy(line_buffer[lc++],":MEASURE:VOLT?");
			//strcpy(line_buffer[lc++],"*STOP");
			strcpy(line_buffer[lc++],":MEASURE:TEMP? 2");
	 int nrmsg=lc;
	 if (nrmsg > NrCmd) {
		 printf("more messages defined than fits in array \n\r");
	 	 nrmsg=NrCmd;
	 }
	 int ttcnt=0;
	 printf("init messages done \n\r");
	 while (ttcnt < 50000) {
		lc=0;
			while(lc< nrmsg) {
				
				tcflush(fd, TCIOFLUSH);
				strcpy(buffer,line_buffer[lc] );
				valread=(int)strlen(buffer);
				buffer[valread++]='\r';
				buffer[valread++]='\n';
				buffer[valread]='\0';
				int nrwr=write(fd,buffer,valread);	
				buffer[valread-2]='\0';//suppress the newline in the output 
				printf("msg nr %02d/%04d len %02d=%02d %-20s  ",lc,ttcnt++,valread,nrwr,buffer);
				valread=read(fd, buffer , sizeof(buffer));//get full return message 
				if( valread == 0) {
					strcpy(buffer,"resp message is zerro");
				} else { 		
					buffer[valread-1]='\0';	// messages is not terminated with \0 !!! overwrite newline
				}
				printf(" resp %-*s len %02d \r\n",18,buffer, (int)strlen(buffer));				
				buffer[0]='\0';
				usleep(10000);
                lc++;
			}

    }
  close (fd) ;
  return 0;
}
