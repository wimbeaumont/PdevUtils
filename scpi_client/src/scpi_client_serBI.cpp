/** 
 * Name        : scpiclient_serBI.cpp
 * Author      : Wim
 * Version     :
 *  Description : program to test a SCPI device connected to a serial port
 *  This version is specific for the Burnin Box @ Brussels for the CMS tracker  
 *  (C) Wim Beaumont Universiteit Antwerpen 2019 2020
 * License see
 *  https://github.com/wimbeaumont/PeripheralDevices/blob/master/LICENSE
 *  Version history :
 *  0.1  copied from scpiclient 
 *  1.2  added close ,  corrected message  '/0' pos
 *  1.3  forked from scpiclient_serB 
 *       added time , for communication ask only the necessary parameters 
 *  1.4  changes,  comments more as stand alone program , time print format corretions 
 
 */ 

#define SCPICLIENSERBI "1.4"


#include <cstdio>
#include <errno.h>  
#include <fcntl.h>  // File Control Definitions for serial         
#include <termios.h>// POSIX Terminal Control Definitions for serial 
#include <cstdlib>     // atof 
#include <unistd.h> 		// for sockets , serial 
#include <string.h> 
#include <ctime>


int serialportsetup(char *portname) {
char SERPORTDEFAULT[]= "/dev/ttyACM1";	

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
	printf("%s ver %s\n\r",argv[0], SCPICLIENSERBI );
//serial port 
	int fd  = serialportsetup(portname);
	if ( fd == -1 ) exit(-1); 

/*  break gives broken pipe and them MBED is stuck 
// send break 
	if ( tcsendbreak( fd,0)) {
		printf( " break fails %s \n", strerror(errno)); 
	} else {
		printf("break sent \n");
	}
	
 sleep(5);
*/
	
// flush port 
	if ( tcflush(fd, TCIOFLUSH)) {
		printf( " flush fails %s \n", strerror(errno)); 
	} else {
		printf("flush done  \n");
	}


	printf("%s ver %s\n\r",argv[0], SCPICLIENSERBI);
	int lc=0;

			strcpy(line_buffer[lc++],":MEAS:TEMP0?");
			strcpy(line_buffer[lc++],":MEAS:TEMP1?");
			strcpy(line_buffer[lc++],":MEAS:TEMP2?");
			strcpy(line_buffer[lc++],":MEASURE:HUMI?");
			strcpy(line_buffer[lc++],":MEASURE:LUMINOSITY?");
	 int nrmsg=lc;
	 if (nrmsg > NrCmd) {
		 printf("more messages defined than fits in array \n\r");
	 	 nrmsg=NrCmd;
	 }
	 int ttcnt=1;
	 printf("init messages done \n\r");
	 while (ttcnt ) { // < 50000) {
		lc=0;
		time_t now = time(0); tm *ltm = localtime(&now);
		printf("%4d%02d%02d %02d:%02d ",1900+ltm->tm_year,1+ltm->tm_mon,ltm->tm_mday,ltm->tm_hour,ltm->tm_min); 	
		while(lc< nrmsg) {
				tcflush(fd, TCIOFLUSH);
				strcpy(buffer,line_buffer[lc] );
				valread=(int)strlen(buffer);
				buffer[valread++]='\r';
				buffer[valread++]='\n';
				buffer[valread]='\0';
				int nrwr=write(fd,buffer,valread);	
				buffer[valread-2]='\0';//suppress the newline in the output 
				//printf("msg nr %03d/%04d len %02d=%02d %-20s ",lc,ttcnt,valread,nrwr,buffer);
				//printf("%-20s ",buffer);
				valread=read(fd, buffer , sizeof(buffer));//get full return message 
				if( valread == 0) {
					strcpy(buffer,"resp message is zerro");
				} else { 		
					buffer[valread-1]='\0';	// messages is not terminated with \0 !!! overwrite newline
				}
				buffer[(int)strlen(buffer)-1]='\0';  //supress the  \r 
				//printf(" resp nr %d %-*s len %03d %03d \r\n",ttcnt++,18,buffer,valread, (int)strlen(buffer));
				printf("%-*s  ",12,buffer);				
				buffer[0]='\0';
				usleep(6000000);
                lc++;
			}
			printf("\r\n");
    }
  close (fd) ;
  return 0;
}
