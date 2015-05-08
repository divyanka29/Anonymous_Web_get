This is the program for anonymous web get.

We have 2 program files namely: awget.c and ss.c and 1 header file: awget.h

AWGET
This program shall get the file from the url specified by the user. 
The requirement is as follows:
1. awget.h, awget.c, makefile and chaingang.txt should be in the same folder.
2. The format of chain file should be as follows:
	a. It should be a text file.
	b. Its contents should be :
	  <Number of ss>
	  <IP1,PORT1>
	  <IP2,PORT2>
	  <IP3,PORT3>
	   .
	   .


Method of Execution
1. Open the terminal and go to the above directory containing all the program files as the present working directory.
2. Execute the command 'make clean'. This will remove all the previous executibles.
3. Execute the command 'make awget' to compile the awget.c and create the executible named awget.
4. Run './awget <url> -c <chain file.txt>'.
5. The -c flag and its argument is optional.
6. The file specified by url will be downloaded to the folder.
7. The url format should be https://www.XXXXX (http format is essential.)

SS
This program shall run the stepping stone. 
The requirement is as follows:
1. awget.h, ss.c, makefile should be in the same folder.

Method of Execution
1. Open the terminal and go to the above directory containing all the program files as the present working directory.
2. Execute the command 'make clean'. This will remove all the previous executibles.
3. Execute the command 'make ss' to compile the ss.c and create the executible named ss.
4. Run './ss <port number>'.
5. The ss will run given port and will wait on listen mode.
6. Once it gets connection from either awget or another ss, it will process it and again return to listen mode.

About running of the program:

Note: The programm is well running for all the conditions for multiple ss. The url format should be https://www.XXXXX (http format is essential.)


