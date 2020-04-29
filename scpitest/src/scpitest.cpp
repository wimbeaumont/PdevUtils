//============================================================================
// Name        : scpitest.cpp
// Author      : Wim
// Version     :
// Copyright   : Your copyright notice
// Description :
//============================================================================
#include <stdio.h>
#include "env_scpiparser.h"
#include <string.h>
int main() {
	const int NrCmd=6;
	char line_buffer[NrCmd][256];
	unsigned char read_length;
	bool stay = true;
	scpi_setup();
	int lc=0;

			strcpy(line_buffer[lc++],"*IDN?");
			strcpy(line_buffer[lc++],":MEAS:TEMP?:CH 1");
			strcpy(line_buffer[lc++],":MEAS:TEMP?");
			strcpy(line_buffer[lc++],":ONZIN:TEMP?");
			strcpy(line_buffer[lc++],":MEASURE:HUMI?");
			strcpy(line_buffer[lc++],":MEASURE:LUMINOSITY?");

	 lc=0;
	while (NrCmd-lc) {

		/* Read in a line and execute it. */
		// read_length = Serial.readBytesUntil('\n', line_buffer, 256);
		//scanf("%s", line_buffer);

		printf("requested cmd %d : %s ",lc,line_buffer[lc]);
		read_length= (unsigned char) strlen(line_buffer[lc]);
		if (read_length > 0) {

					env_scpi_execute_command( line_buffer[lc], read_length);


				printf("got , %s\n\r", env_get_result() );
		}
		lc++;
	}
}

