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
po::variables_map process_program_options(const int argc, const char* const argv[])
{
    po::options_description desc("Allowed options");

    desc.add_options()("help,h", "produce this help message")
                      ("serialport,s",po::value<string>()->default_value("/dev/ttyACM1"),"set serial port to connect to")
                      ("waittime,w", po::value<int>()->implicit_value(120), "time to wait between two measurements [s]");

    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
    }
    catch(po::error const& e)
    {
        std::cerr << e.what() << '\n';
        exit(EXIT_FAILURE);
    }
    po::notify(vm);

    // Help
    if(vm.count("help"))
    {
        cout << desc << "\n";
        exit(EXIT_SUCCESS);
    }

    return vm;
}







//============================================================================

int main(int argc, char const *argv[]){
	int valread; 
    char buffer[256];
    const int NrCmd=20;
    char line_buffer[NrCmd][256];
    
    
    po::variables_map v_map = process_program_options(argc, argv);
    std::string        portname  = v_map["serialport"].as<string>();
    int waittime =  v_map["waittime"].as<int>();
    if (waittime < 5 )  waittime=10;
	printf("%s ver %s\n\r",argv[0], SCPICLIENTSERBI );
    //serial port 
	int fd  = serialportsetup(portname);
	if ( fd == -1 ) exit(-1);
	
    std::ofstream outfile; 
    outfile.open("/data/logs/mbed.log", std::ios_base::app);
	if ( outfile.fail()) exit(-3);
 

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
		std::stringstream ss;
		 
		lc=0;
		time_t now = time(0); tm *ltm = localtime(&now);
		ss << std::setfill('0') << std::setw(4) << 1900+ltm->tm_year;
		ss <<std::setw(2) <<1+ltm->tm_mon<<std::setw(2) <<ltm->tm_mday<<"  "<<std::setw(2) <<ltm->tm_hour<<":"<<std::setw(2) <<ltm->tm_min<<" ";
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
				ss << buffer <<" " ;
				
				//printf("%-*s  ",12,buffer);				
				buffer[0]='\0';
				//usleep(waittime * 10000);
				usleep(1000000);
                lc++;
			}
			ss <<endl;
			outfile <<ss.str();
			outfile.flush();
			cout << ss.str();
			sleep(waittime);
    }
  close (fd) ;
  return 0;
}
