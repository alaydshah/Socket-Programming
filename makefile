CC = g++
CFLAGS = -std=c++11

default: all

all:  
	$(CC) $(CFLAGS) -o client client.cpp
	$(CC) $(CFLAGS) -o scheduler scheduler.cpp server.cpp
	$(CC) $(CFLAGS) -o hospitalA hospitalA.cpp hospital.cpp graph.cpp server.cpp
	$(CC) $(CFLAGS) -o hospitalB hospitalB.cpp hospital.cpp graph.cpp server.cpp
	$(CC) $(CFLAGS) -o hospitalC hospitalC.cpp hospital.cpp graph.cpp server.cpp

clean: 
	$(RM) client scheduler hospitalA hospitalB hospitalC