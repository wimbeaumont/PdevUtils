//============================================================================
// Name        : scpiclient.cpp
// Author      : Wim
// Version     :
// Copyright   : Willem Beaumont University Antwerpen
// Description : program to send a command to scpi_client_serBI 
// V1.0 : initial version  20251202
// V1.1 : tested version
// V2.0 : aded cmd line options forthe moment only HVon HVoff
//============================================================================

#define SER_BI_CMND "2.0"

// Client side C/C++ program to demonstrate Socket programming
#include <cstdio>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#define PORT 9090
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
// Namespaces
using namespace std;
namespace po = boost::program_options;



/*!
************************************************
* Argument parser.
************************************************
*/
po::variables_map process_program_options(const int argc, const char* const argv[]){
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce this help message")
                      ("HVset,H",po::value<string>(),"to send HVon or HVoff --HVset on");
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



int main(int argc, char const *argv[]){
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[256];
    char line_buffer[256];
    //socket setup stuff 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    po::variables_map v_map = process_program_options(argc, argv);
    if ( ! v_map.count("HVset")) {
        cout << "HVset argument is required try -h " << endl;
        exit(-1);
    }
    std::string  HvSet  = v_map["HVset"].as<string>();
    boost::algorithm::to_upper(HvSet);
    if  ( HvSet == "ON") {  HvSet = "HVon";}
    else if ( HvSet == "OFF")  {        HvSet = "HVoff";} 
    else {
        cout << "HVset should be On or Off " << endl;
        exit(-1);
    }
	int lc=0;
    strcpy(line_buffer,HvSet.c_str());
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)       {
             printf("\n Socket creation error \n");
             return -1;
         }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
         {
             printf("\nConnection Failed is the server running ?\n");
             return -1;
         }
    valread=send ( sock , line_buffer, strlen(line_buffer) , 0 );
    printf("sent msg nr %d %s length %d sould be %d \n\r",lc,line_buffer,(int)strlen(line_buffer) ,valread);
    buffer[0]='\0';
    valread = read( sock , buffer, sizeof(buffer));
    buffer[valread]='\0';
    printf("got (length %d,%d):%s\n",(int) strlen(buffer),valread,buffer );
    close(sock);
    
          lc++;

 
  return 0;
}
