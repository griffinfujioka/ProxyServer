GRIFFIN FUJIOKA 
WSU ID: 11044124
FALL 2013, WASHINGTON STATE UNIVERSITY 
COURSE: CPTS 455
INSTRUCTOR: CARL HAUSER


Contained within is an implementation of a forking proxy server. Some the files in this directory are as follows: 

DieWithMessage.c - Contains helper functions for gracefully exiting program upon an error. 

connect_example.c - Sub-program for converting a hostname to an IP address and then creating a socket which connects to the IP address. 

hostname_helpers.c - Contains functions pertaining to host names. 

nonforking_proxy.c - A nonforking implementation of the proxy server used as a base project for understanding how each thread created in the forking project will behave. 

proxy.c - A forking proxy server. 


HOW TO COMPILE: 
Compile files using the following pattern: 

gcc [filename].c -o [filename]

Examples: 
gcc connect_example.c -o connect_example

gcc proxy.c -o proxy 



