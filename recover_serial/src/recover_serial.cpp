/** 
 * Name        : recover_serial.cpp
 * Author      : Wim
 * Version     :
 *  Description : program to check reovering the communication of a serial port 
 *  when a pico ( and most likely other uProcessors) reboot ( by example Watch Dog) the communication is lost
 *  a simple try on the read seems not to work as somehow the program doesn't see the read is failing
 *  so it checks the number of bytes received if this is 0 ( although the read is blocking) it ask the status of 
 *  the serial port  and reinitialize the serial port.  
 *   *  (C) Wim Beaumont Universiteit Antwerpen 2026
 * License see
 *  https://github.com/wimbeaumont/PeripheralDevices/blob/master/LICENSE
 *  Version history :
 *  1.0    copied serialportset  from scpiclient  tested with Raspberry pico 
 *         with the firmware Wathc_dog (github.com/wimbeaumont/picopro , not released when writing this info) 
 */ 




#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <cstdio>
#include <errno.h>  
#include <fcntl.h>  // File Control Definitions for serial         
#include <termios.h>// POSIX Terminal Control Definitions for serial 
#include <cstdlib>     // atof 
#include <string.h> 
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream> 
#include <boost/program_options.hpp>
// Namespaces
using namespace std;
namespace po = boost::program_options;


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
                      ("serialport,s",po::value<string>()->default_value("/dev/ttyACM0"),"set serial port to connect to");
                      //("outputfile,o",po::value<string>()->default_value("/tmp/pico.log"), "file for storing the results")
                      //("waittime,w", po::value<int>()->default_value((int)110), "time to wait between two measurements [s]");

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


//============================================================================

int main(int argc, char const *argv[]){
	
    char buffer[500];
    int valread;
    po::variables_map v_map = process_program_options(argc, argv);
    std::string  portname  = v_map["serialport"].as<string>();
    //std::string  outputfilename  = v_map["outputfile"].as<string>();
    //int waittime=  v_map["waittime"].as<int>();
    
    //serial port 
	int fd  = serialportsetup(portname);
	if ( fd == -1 ) { printf( " init serial port %s failed \n" , portname.c_str());
		exit(-1);
	}
	uint buffersize=sizeof(buffer);
	int cnt=0;
	
	while(1) {
		valread=read(fd, buffer , buffersize);//get full return message 	
		if( valread > 0) {
			// the following was needed because the pico send two \n  ,  solved by  supressing the automatic CR to LF conversion
			//if (valread==1){ // seems the new line is not taken by the read
			//	printf("valread is 1 print %d\n",(int) buffer[0]);
			//	continue;
			//}
			//int lastchar=(int)buffer[valread-1];
			buffer[valread-1]='\0'; // got some extra char and strip new line
			//printf("valread at line %d : %d , strlength %d last char %d \n",__LINE__, valread,(int) strlen(buffer),lastchar);	
			printf ("got : %s \n",buffer);
		}
		else {
			printf("enter else valread %d   ",valread);
			struct stat fstatfd;
		    int fstat_ret =fstat(fd , &fstatfd);
		    if (fstat_ret ==-1 ) {
				printf("lost communication %s\n", strerror(errno));
				close(fd) ;
				fd  = serialportsetup(portname);
				if( fd < 0 ) sleep(1);
				
			} else { 
				printf ("fstat check returned %d amd fd is %d %d\n ", fstat_ret,fd, cnt++);
				//sleep(1);
				if ( cnt > 30) {
				    cnt=0;
					close(fd) ;
					fd  = serialportsetup(portname);
				}	
			}
			
		}
		
	}
}
