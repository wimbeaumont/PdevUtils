# PdevUtils

some utility code that could be useful when using the PeripheralDevice classes 

Often applications that use sensors need implementations that are common.  Code of this kind I have collected here. So this code is not needed for the Peripheral classes and or otherway around. 

For some of the applications it is, mainly because of organizational reasons ( independent developments) it is easier to have a standalone program that readout the sensors and then communicate with the main program.  

The MBED platform is an other  example where the software to read the sensors is standalone. 

So there need some communication.  In the past I have used a own  ( asci based ) communication protocol.  But  now I decided to adopt SCPI as  protocol. 

So there is a library scpi-parser that interpret the SCPI commands and  then connect it to an action. The library is based on https://github.com/LachlanGunn/oic   from Lachlan Gunn  for Ardriono.   I try to keep this much as possible the same.  The main difference is that the  lib  of Lachlan Gunn    is using directly  the serial interface to communicate while this library use internal a string that then can be return via the chosen communication interface.  The footprint of the code in the processor memory will be bigger. This is not a real issue for the  Raspberry Py what I only used as target.  So not clear what is the overhead for the MBED platforms. 



This project can be found  on github 

https://github.com/wimbeaumont/PdevUtils


