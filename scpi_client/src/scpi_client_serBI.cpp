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
 *  1.5  added string stream and storage of log file 
 *  1.6  moved wait time from between measurements to between all measurements
 *  1.7  optimized for pico
 *  1.8  added socket 
 * 
 
 */ 

#define SCPICLIENTSERBI "1.6"


#include <cstdio>
#include <errno.h>  
#include <fcntl.h>  // File Control Definitions for serial         
#include <termios.h>// POSIX Terminal Control Definitions for serial 
#include <cstdlib>     // atof 
#include <unistd.h> 		// for sockets , serial 
#include <string.h> 
#include <ctime>
#include <sys/socket.h> // for sockets
#include <netinet/in.h> // for sockets
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream> 
#include <boost/program_options.hpp>
// Namespaces
using namespace std;
namespace po = boost::program_options;

#define PORT 9090 //port for socket communication

int serialportsetup(string portname) {

printf("\n  try to open %s\n",portname.c_str());
int fd=open(portname.c_str(),O_RDWR | O_NOCTTY); //fd is locally defined but the open is static so also have to be closed

	if(fd ==-1)
     printf("\n  Error! in Opening %s\n",portname.c_str());
	else{ 
		printf("\n  %s Opened Successfully\n",portname.c_str());
		
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


/*!
************************************************
* Argument parser.
************************************************
*/
po::variables_map process_program_options(const int argc, const char* const argv[]){
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce this help message")
                      ("serialport,s",po::value<string>()->default_value("/dev/ttyACM0"),"set serial port to connect to")
                      ("outputfile,o",po::value<string>()->default_value("/tmp/pico.log"), "file for storing the results")
                      ("waittime,w", po::value<int>()->default_value((int)110), "time to wait between two measurements [s]");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
    }
    catch(po::error const& e)  {
        std::cerr << e.what() << '\n';
        exit(EXIT_FAILURE);
    }
    po::notify(vm);
    // Help
    if(vm.count("help"))   {
        cout << desc << "\n";
        exit(EXIT_SUCCESS);
    }
    return vm;
}


void  get_info (int fd, const char* request , char* buffer, int buffersize ) {
	int valread; 
	tcflush(fd, TCIOFLUSH);
	strcpy(buffer,request );
	valread=(int)strlen(buffer);
	buffer[valread++]='\r';buffer[valread++]='\n';buffer[valread]='\0';//endl added 
	int nrwr=write(fd,buffer,valread);	
	//buffer[valread-2]='\0';//suppress the newline in the output 
	//printf("msg nr %03d/%04d len %02d=%02d %-20s ",lc,ttcnt,valread,nrwr,buffer);
	//printf("%-20s ",buffer);
	valread=read(fd, buffer , buffersize);//get full return message 	
	//printf("valread at line %d : %d , buffersize %d \n\r",__LINE__, valread, buffersize);	
	if( valread == 0) {
		strcpy(buffer,"resp message is zerro");
	} else { 		
		buffer[valread]='\0';	// messages is not terminated with \0 !!! overwrite newline 
		//20251025  now seems to be valread instead of valread -1 
	}
	buffer[(int)strlen(buffer)-1]='\0';  //supress the  \r 
	//printf("Buf: %-*s  ",12,buffer);				
	//printf("Buf: %s  \n\r",buffer);				
}

int set_socket_bindings(int server_fd){
    struct sockaddr_in address; 
    int addrlen = sizeof(address); 
    int opt = 1;   
    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt)))  { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = htonl(INADDR_ANY); 
    address.sin_port = htons( PORT ); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
	// make it non blocking
	int flags = fcntl(server_fd, F_GETFL, 0);
	if (flags == -1) {
		perror("fcntl F_GETFL");
		exit(EXIT_FAILURE);
	}
	if(fcntl(server_fd, F_SETFL, flags | O_NONBLOCK)== -1) {
		perror("fcntl F_SETFL");
		exit(EXIT_FAILURE);
	}
	if(listen(server_fd, 3) <0 ) {
		perror("socket listen failed ");
		return -1;
	}
	// socket setup done 	
	return 0;
}


int new_socket ;
int  get_socket_message(int server_fd, char* buffer, int buffersize){
	
	new_socket = accept(server_fd, NULL, NULL);
	if( errno == EWOULDBLOCK || errno == EAGAIN) {
		// no client connected 
		return 0;
	} else if ( new_socket <0) {
		perror("socket accept failed ");
		return -1;
	}
	
	ssize_t  valread = read( new_socket, buffer, buffersize); 
	if ( valread == 0 ) {
		printf("Socket closed by client \n");
		return -1;
	} else if ( valread >= buffersize ) {
		//message too long					
		valread = buffersize -1;
	}
	buffer[valread] = '\0'; 
	return (int)valread;
	//printf("Socket msg received: %s\n",buffer ); 
}

int interprete_socket_message(int serial_fd , int server_fd, char* buffer, int buffersize){
	int valread= get_socket_message(server_fd, buffer, buffersize);
	if ( valread <=0) { printf( " get %d back from socketreturn", valread);return valread; }// no message or error
	printf( " there is a message\n");
	if ( strncmp(buffer,"HvOn",4) ==0) {
		get_info(serial_fd,":SET:IO:CH 0 1", buffer, buffersize	);
	} else if ( strncmp(buffer,"HvOff",5) ==0) {
		get_info(serial_fd,":SET:IO:CH 0 0", buffer, buffersize	);	
		buffer=strcpy(buffer,"OK\r\n");
	}else { cout << "Unknown socket command : " << buffer << endl;
	  buffer=strcpy(buffer,"NotOK\r\n");	
	}
	
	send(new_socket , buffer , strlen(buffer) , 0 );
	close(new_socket);
	cout << "Socket command processed : " << buffer << endl;
	return valread;
}

//============================================================================

int main(int argc, char const *argv[]){
	int valread; 
    char buffer[256];
    const int NrCmd=20;
    char line_buffer[NrCmd][256];
    
    
    po::variables_map v_map = process_program_options(argc, argv);
    std::string  portname  = v_map["serialport"].as<string>();
    std::string  outputfilename  = v_map["outputfile"].as<string>();
    int waittime=  v_map["waittime"].as<int>();
    if (waittime < 2 )  waittime=2;
	printf("%s ver %s  wait time %d \n\r",argv[0], SCPICLIENTSERBI, waittime );
    //serial port 
	int fd  = serialportsetup(portname);
	if ( fd == -1 ) exit(-1);
	
	// setup the socket  simple socket for 1 client
    int server_fd; 

   // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
	set_socket_bindings(server_fd);
	
    std::ofstream outfile;
     
    outfile.open(outputfilename, std::ios_base::app);
	if ( outfile.fail()) {
		printf("opening  outputfile %s failed \n\r" , outputfilename.c_str());
		exit(-3);
	}

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
	int lc=0;
			//strcpy(line_buffer[lc++],":SET:IO:CH 0 1");
			//strcpy(line_buffer[lc++],":SET:IO:CH 1 1");
			strcpy(line_buffer[lc++],":MEAS:VOLT?:CH 0");
			strcpy(line_buffer[lc++],":MEAS:TEMP0?");
			strcpy(line_buffer[lc++],":MEAS:TEMP1?");
			strcpy(line_buffer[lc++],":MEAS:TEMP3?");
			strcpy(line_buffer[lc++],":MEASURE:HUMI?");
			strcpy(line_buffer[lc++],":MEASURE:LUMINOSITY?");
			strcpy(line_buffer[lc++],":GET:STATUS?");
			

	 int nrmsg=lc;
	 if (nrmsg > NrCmd) {
		 printf("more messages defined than fits in array \n\r");
	 	 nrmsg=NrCmd;
	 }
	 int ttcnt=1;
	 printf("init messages done \n\r");
	 get_info (fd, "*IDN?" , buffer,sizeof(buffer) ) ;	 
	 printf(" this is %s \n\r", buffer);
	 get_info (fd, "*HWVer?" , buffer,sizeof(buffer) ) ;	 
	 printf(" this is HWVer %s \n\r", buffer);
	 get_info (fd, ":GET:STATUS?" , buffer,sizeof(buffer) ) ;	 
	 printf("status  %s \n\r", buffer);
	 get_info (fd, ":SET:IO:CH 1 0" , buffer,sizeof(buffer) ) ;	 
	 printf(" io is %s \n\r", buffer);
	 int toggle=0;
	 while (ttcnt ) { // < 50000) {
		std::stringstream ss;		 
		lc=0;
		time_t now = time(0); tm *ltm = localtime(&now);
		ss << std::setfill('0') << std::setw(4) << 1900+ltm->tm_year;
		ss <<std::setw(2) <<1+ltm->tm_mon<<std::setw(2) <<ltm->tm_mday<<"  "<<std::setw(2) <<ltm->tm_hour<<":"<<std::setw(2) <<ltm->tm_min<<" ";
		while(lc< nrmsg) {
		/*		if (lc ==0) {
					if ( toggle ==0) {
						get_info (fd, ":SET:IO:CH 0 1" , buffer,sizeof(buffer) ) ;	 
						toggle=1;
					} else {
						get_info (fd, ":SET:IO:CH 0 0" , buffer,sizeof(buffer) ) ;	 
						toggle=0;
					}
				}
				else {	*/
				interprete_socket_message(fd, server_fd, buffer, sizeof(buffer));
				get_info (fd, line_buffer[lc] , buffer,sizeof(buffer) ) ;
				//printf(" resp nr %d %-*s len %03d %03d \r\n",ttcnt++,18,buffer,valread, (int)strlen(buffer));
				ss << buffer <<" " ;
				
				//printf("%-*s  ",12,buffer);				
				buffer[0]='\0';
				usleep(5* 10000); // fix value , see what is optimal 
				//usleep(1000000);
                lc++;
			}
			ss <<endl;
			outfile <<ss.str();
			outfile.flush();
			std::cout << ss.str();
			int  slept=sleep(waittime);
			//cout << "waittime of "<<waittime <<" slept " << slept <<"  ends" << endl;
    }
  close (fd) ;
  return 0;
}
