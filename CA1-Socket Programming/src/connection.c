#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

#include "connection.h"
#include "besides.h"
#include "loger.h"


int makeBroadcast(const char* ipAddr, unsigned short port, struct sockaddr_in* addrOut) {
    int socketId = socket(PF_INET, SOCK_DGRAM, 0);
    if (socketId < 0) 
        return socketId;

    int broadcast = 1;
    int reuseport = 1;
    setsockopt(socketId, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(socketId, SOL_SOCKET, SO_REUSEPORT, &reuseport, sizeof(reuseport));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ipAddr, &(addr.sin_addr.s_addr));
    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

    bind(socketId, (struct sockaddr*)&addr, sizeof(addr));
    *addrOut = addr;
    return socketId;
}

int makeServer(unsigned short port) {
    int socketId = socket(PF_INET, SOCK_STREAM, 0);
    if (socketId < 0) return socketId;

    int opt = 1;
    setsockopt(socketId, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

    bind(socketId, (struct sockaddr*)&addr, sizeof(addr));
    listen(socketId, 4);
    return socketId;
}

int acceptClient(int serverFd) {
    int clientFd;
    struct sockaddr_in clientAddress;
    int addressLen = sizeof(clientAddress);
    
    clientFd = accept(serverFd, (struct sockaddr *)&clientAddress, (socklen_t*) &addressLen);

    return clientFd;
}

int connectServer(unsigned short port, int* outServerSocket) {
    int serverId = socket(PF_INET, SOCK_STREAM, 0);
    if (serverId < 0) return serverId;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &(addr.sin_addr.s_addr));
    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

    *outServerSocket = serverId;
    return connect(serverId, (struct sockaddr*)&addr, sizeof(addr));
    
}

void FD_SETTER(int socketId, FdSet* fdset) {
    FD_SET(socketId, &fdset->masterSet);
    if (socketId > fdset->maxFd) 
        fdset->maxFd = socketId;
}

void FD_CLEARER(int socketId, FdSet* fdset) {
    FD_CLR(socketId, &fdset->masterSet);
    if (socketId == fdset->maxFd)
        --fdset->maxFd;
}

int sendBcast(struct sockaddr_in bcastAddr,int bcastSocket, const char* msg){
    return sendto(bcastSocket, msg, strlen(msg), 0, (struct sockaddr*)&bcastAddr, sizeof(bcastAddr));
}

void sendWantedInformation(int port,char* msg){
    int connectSocket;
    int result=connectServer(port,&connectSocket);
    if(result<0){
        logError("Failed to connect to server for send information");
        return ;
    }
    write(connectSocket,msg,strlen(msg));
    close(connectSocket);
}