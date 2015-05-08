CC=gcc
CFLAGS= -Wall
THREADFLAG = -lpthread
FILE1 = awget
FILE2 = ss

awget:
	$(CC) $(CFLAGS) -o $(FILE1) $(FILE1).c

ss:
	$(CC) $(CFLAGS) $(THREADFLAG) -o $(FILE2) $(FILE2).c

all: awget ss

clean:
	rm -f $(FILE1) $(FILE2)
