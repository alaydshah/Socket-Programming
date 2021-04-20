CC = g++
CFLAGS = -std=c++11

default: all

all:  
	$(CC) $(CFLAGS) -o client client.cpp
	$(CC) $(CFLAGS) -o scheduler scheduler.cpp Server.cpp
	$(CC) $(CFLAGS) -o hospitalA hospitalA.cpp Hospital.cpp Graph.cpp Server.cpp
	$(CC) $(CFLAGS) -o hospitalB hospitalB.cpp Hospital.cpp Graph.cpp Server.cpp
	$(CC) $(CFLAGS) -o hospitalC hospitalC.cpp Hospital.cpp Graph.cpp Server.cpp

clean: 
	$(RM) client scheduler hospitalA hospitalB hospitalC