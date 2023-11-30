#ifndef CONNECTION_H_INCLUDE
#define CONNECTION_H_INCLUDE

#include <netinet/in.h>

#include "defs.h"

int makeBroadcast(const char* ipAddr, unsigned short port, struct sockaddr_in* addrOut);
int makeServer(unsigned short port);
int acceptClient(int serverFd);
int connectServer(unsigned short port, int* outServerSocket);

void FD_SETTER(int socket, FdSet* fdset);
void FD_CLEARER(int socket, FdSet* fdset);

int sendBcast(struct sockaddr_in bcastAddr,int bcastSocket, const char* msg);
void sendWantedInformation(int port,char* msg);



#endif