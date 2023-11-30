#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>

#include "loger.h"
#include "defs.h"
#include "connection.h"
#include "besides.h"
#include "ansi-color.h"


Supplier spr;


void getReqInfo(int clientSocket,FdSet *fdSet){
    char msg[MAX_MSG];
    if(read(clientSocket,msg,MAX_MSG)==0){
        spr.haveRequest=NO;
        FD_CLEARER(clientSocket,fdSet);
        logWarning("Request timeout");
        logMsg(spr.username,"Time to answer to request end.");
        return;
    }
    if(spr.haveRequest==YES){
        logMsg(spr.username,"A request was rejected because already have one.");
        sprintf(msg,"full");
        write(clientSocket,msg,MAX_MSG);
        close(clientSocket);
        FD_CLEARER(clientSocket,fdSet);
        return;
    }
    spr.haveRequest=YES;
    sprintf(msg,"%s new %s request ingredient\n",ANSI_GRN,ANSI_RST);
    write(STDOUT_FILENO,msg,strlen(msg));
    spr.clientSocket=clientSocket;
    logMsg(spr.username,"Get a new request ingredient successfully.");
}

void answerRequest(FdSet *fdSet){
    char msg[MAX_MSG];
    if(spr.haveRequest==NO){
        logWarning("No request in line.");
        return;
    }
    sprintf(msg,"-> your answer (%syes%s/%sno%s): ",ANSI_GRN,ANSI_RST,ANSI_RED,ANSI_RST);
    getInput(STDIN_FILENO,msg, spr.requestAnswer, NAME_SIZE);
    sprintf(msg,"%s %s", spr.requestAnswer,spr.username);
    if(!strcmp(spr.requestAnswer, "yes")){
        write(spr.clientSocket,msg,MAX_MSG);
        logMsg(spr.username,"Request ingredient accept successfully.");
    }
    else if(!strcmp(spr.requestAnswer, "no")){
        write(spr.clientSocket,msg,MAX_MSG);
        logMsg(spr.username,"Request ingredient reject successfully.");
    }
    spr.haveRequest=NO;
    FD_CLEARER(spr.clientSocket,fdSet);
    close(spr.clientSocket);
    logMsg(spr.username,"Close tcp port successfully.");
}


void brodCastAction(char* brodcBuff){
    char infoMsg[BCAST_MSG];
    char *brodcPart=tokenizeStr(brodcBuff,DELIMETER);
    if(!strcmp(brodcPart,"suppliers")){
        int port=strToPort(tokenizeStr(brodcBuff,DELIMETER));
        sprintf(infoMsg,"%s %s %s %s %d %s",ANSI_YEL,spr.username,ANSI_RST,ANSI_MAG,spr.tcpPort,ANSI_RST);
        sendWantedInformation(port,infoMsg);
        logMsg(spr.username,"Send name and port to someone requested successfully.");
    }
    else if(!strcmp(brodcPart,"username")){
        brodcPart=tokenizeStr(brodcBuff,DELIMETER);
        if(!strcmp(brodcPart,spr.username)){
            sprintf(infoMsg,DUP_ERROR);
            sendBcast(spr.bcast.addr,spr.bcast.bcastSocket,infoMsg);
            logMsg(spr.username,"Send duplicate warning on broadcast successfully.");
        }
    }
    else if(!strcmp(brodcPart,"display")){
        write(STDOUT_FILENO,brodcBuff,strlen(brodcBuff));
        logMsg(spr.username,"Display brodcast message successfully.");
    }
    else{
        logMsg(spr.username,"Recive something on broadcast that is not important.");
    }
}

void commandHandler(FdSet* fdSet){
    char cmdBuf[MAX_BUFF] = {'\0'};
    char msg[MAX_MSG] = {'\0'};
    getInput(STDIN_FILENO, NULL, cmdBuf, MAX_BUFF);
    char* cmdPart1 =tokenizeStr(cmdBuf,DELIMETER);
    char* cmdPart2 =tokenizeStr(cmdBuf,DELIMETER);
    if(cmdPart1==NULL) {
        logError("Unknown command.");
        return;
    }
    if(!strcmp(cmdPart1,"answer")&&!strcmp(cmdPart2,"request")){
        answerRequest(fdSet);
        logMsg(spr.username,"Answer to a request ingredient successfully.");
    }
    else
        logError("Unknown command.");
}
void interface(){
    char msgBuf[MAX_BUFF];
    char brodcBuf[MAX_BUFF]={0};
    FdSet fdSet;
    fdSet.maxFd=0;
    FD_ZERO(&fdSet.masterSet);
    FD_SETTER(STDIN_FILENO, &fdSet);
    FD_SETTER(spr.bcast.bcastSocket, &fdSet);
    FD_SETTER(spr.serverSocket, &fdSet);
    while(1){
        cliPrimary();
        memset(msgBuf, '\0', MAX_BUFF);
        fdSet.workingSet = fdSet.masterSet;
        select(fdSet.maxFd + 1, &fdSet.workingSet, NULL, NULL, NULL);

        for (int i = 0; i <= fdSet.maxFd; ++i) {
            if (!FD_ISSET(i, &fdSet.workingSet)) continue;
            if (i != STDIN_FILENO) {
                write(STDOUT_FILENO, "\x1B[2K\r", 5);
            }
            if(i==STDIN_FILENO){
                commandHandler(&fdSet);
            }
            else if(i==spr.bcast.bcastSocket){
                memset(brodcBuf, 0, 1024);
                recv(i,brodcBuf, 1024, 0);
                logMsg(spr.username,"Recive something on broadcast.");
                brodCastAction(brodcBuf);                
                logMsg(spr.username,"Action on brodcast successfully.");
            }
            else if(i==spr.serverSocket){
                int acceptSock = acceptClient(spr.serverSocket);
                FD_SETTER(acceptSock, &fdSet);
                logMsg(spr.username,"Accept a restaurant");
            }
            else{
                getReqInfo(i,&fdSet);
            }
        }
    }
}


void initBrodcast(const char* port){
    spr.udpPort= strToPort(port);
    spr.bcast.bcastSocket = makeBroadcast(BRODCAST_IP,spr.udpPort, &spr.bcast.addr);
    logInfo("Broadcast receiving successfully initialized.");
}

void initServer(){
    while (1)
    {
        spr.tcpPort=generateRandomPort();
        spr.serverSocket=makeServer(spr.tcpPort);
        if(spr.serverSocket>=0)
            break;
    }
    logInfo("Making server succesfully.");
}

void checkDuplicate(){
    signal(SIGALRM,alarmhandler);
    siginterrupt(SIGALRM,1);
    char responseBuf[BCAST_MSG];
    char brodMsg[BCAST_MSG];
    sprintf(brodMsg,"username %s",spr.username);
    sendBcast(spr.bcast.addr,spr.bcast.bcastSocket,brodMsg);
    while(1){
        int res=-1;
        memset(responseBuf, 0, BCAST_MSG);
        alarm(1);
        res=recv(spr.bcast.bcastSocket,responseBuf,BCAST_MSG,0);
        alarm(0);
        if(res<0)  return;
        if(!strcmp(responseBuf,DUP_ERROR)){
            logError("Duplicate username!");
            getInput(STDIN_FILENO, "Enter your username: ", spr.username, NAME_SIZE);
            sprintf(brodMsg,"username %s",spr.username);
            sendBcast(spr.bcast.addr,spr.bcast.bcastSocket,brodMsg);
        }
    } 
}

void initUsername(){
    getInput(STDIN_FILENO, "Please enter your username: ", spr.username, NAME_SIZE);
    checkDuplicate();
}

void initSupplier(const char* port){
  spr.haveRequest=NO;
  initBrodcast(port);
  initServer();
  initUsername();
  createLogFile(spr.username);
  logMsg(spr.username,"Supplier created successfully."); 
  char welcomeMsg[MAX_MSG];
  sprintf(welcomeMsg,"welcome %s %s %s as supplier\n",ANSI_GRN,spr.username,ANSI_RST);
  write(STDOUT_FILENO,welcomeMsg,strlen(welcomeMsg));
}
int main(int argc, const char* argv[]){
    if (argc != 2) {
        logError("Wrong number of arguments.");
        return EXIT_FAILURE;
    }
    initSupplier(argv[1]);
    interface();
}