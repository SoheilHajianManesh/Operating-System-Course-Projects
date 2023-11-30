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
#include "json.h"
#include "ansi-color.h"

Customer csr;

int alarmstat=0;
void alarmhandler2(int sig){
    logMsg(csr.username,"Alarm handler activate.");
    alarmstat=1;
}

void ordering(){
    char msg[MAX_MSG];
    char response[MAX_MSG];
    char restaurantPort[MAX_PORT_LEN];
    char nameOfFood[NAME_SIZE];
    char infoMsg[MAX_MSG];
    sprintf(infoMsg,"-> name of %sfood%s: ",ANSI_CYN,ANSI_RST);
    getInput(STDIN_FILENO,infoMsg, nameOfFood, NAME_SIZE);
    sprintf(infoMsg,"-> %sPort%s of restaurant: ",ANSI_MAG,ANSI_RST);
    getInput(STDIN_FILENO,infoMsg,restaurantPort, MAX_PORT_LEN);
    int rstPort=strToPort(restaurantPort);
    int connectSocket;
    int result=connectServer(rstPort,&connectSocket);
    if(result<0){
        logError("connection failed");
        return;
    }
    logMsg(csr.username,"connect to restaurant for order successfully.");
    signal(SIGALRM,alarmhandler2);
    siginterrupt(SIGALRM, 1);
    sprintf(msg,"%s %s",nameOfFood,csr.username);
    write(connectSocket,msg,MAX_MSG);
    sprintf(msg,"%s waiting %s for restaurant's response ... \n",ANSI_YEL,ANSI_RST);
    write(STDOUT_FILENO,msg,strlen(msg));
    alarm(120);
    int res=recv(connectSocket,response,MAX_MSG,0);
    alarm(0);
    char* token=tokenizeStr(response,DELIMETER);
    if(alarmstat==1||res==0){
        logError("Request Timeout.");
        logMsg(csr.username,"Request for food timeout.");
    }
    else if(!strcmp(token,"close")){
        logError("Restaurant closed.");
        logMsg(csr.username,"Restaurant closed.");
    }
    else if(!strcmp(token,"yes")){
        token=tokenizeStr(response,DELIMETER);
        sprintf(msg,"%s %s %s restaurant %s accepted %s and your food is ready\n",ANSI_YEL,token,ANSI_RST,ANSI_GRN,ANSI_RST);
        write(STDOUT_FILENO,msg,strlen(msg));
        logMsg(csr.username,"Restaurant accept your order.");
    }
    else if(!strcmp(token,"no")){
        token=tokenizeStr(response,DELIMETER);
        sprintf(msg,"%s %s %s restaurant %s rejected %s and cry about it\n",ANSI_YEL,token,ANSI_RST,ANSI_RED,ANSI_RST);
        write(STDOUT_FILENO,msg,strlen(msg));
        logMsg(csr.username,"Restaurant reject your order.");
    }
    close(connectSocket);
    alarmstat=0;
    logMsg(csr.username,"Connection with restaurant closed successfully.");
}

void brodCastAction(char* brodcBuff){
    char infoMsg[BCAST_MSG];
    char *brodcPart=tokenizeStr(brodcBuff,DELIMETER);
    if(!strcmp(brodcPart,"username")){
        brodcPart=tokenizeStr(brodcBuff,DELIMETER);
        if(!strcmp(brodcPart,csr.username)){
            sprintf(infoMsg,DUP_ERROR);
            sendBcast(csr.bcast.addr,csr.bcast.bcastSocket,infoMsg);
            logMsg(csr.username,"Send duplicate warning on broadcast successfully.");
        }
    }
    else if(!strcmp(brodcPart,"display")){
        write(STDOUT_FILENO,brodcBuff,strlen(brodcBuff));
        logMsg(csr.username,"Display brodcast message successfully.");
    }
    else{
        logMsg(csr.username,"Recive something on broadcast that is not important.");
    }
}

void showRestaurants(){
    char msg[BCAST_MSG];
    char response[BCAST_MSG]={'\0'};
    signal(SIGALRM,alarmhandler);
    siginterrupt(SIGALRM,1);
    sprintf(msg,"username/port\n");
    write(STDOUT_FILENO,msg,strlen(msg));
    sprintf(msg,"restaurants %d",csr.tcpPort);
    sendBcast(csr.bcast.addr,csr.bcast.bcastSocket,msg);
    logMsg(csr.username,"Sent Wanting restaurants on brodcast successfully.");
    while(1){
        int accSocket=-1;
        alarm(1);
        accSocket=acceptClient(csr.serverSocket);
        alarm(0);
        if(accSocket<0){ 
            break;
        }
        memset(response, 0, BCAST_MSG);
        recv(accSocket,response,BCAST_MSG,0);
        write(STDOUT_FILENO,response,strlen(response));
        write(STDOUT_FILENO,"\n",1);
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
    if(!strcmp(cmdPart1,"show")){
        if(!strcmp(cmdPart2,"restaurants")){
            showRestaurants();
            logMsg(csr.username,"Restaurants shown successfully.");
        }
        else if(!strcmp(cmdPart2,"menu")){
            Foods foods;
            getFoods(&foods);
            showFoods(&foods);
            logMsg(csr.username,"Menu shown successfully.");
        }
        else {
            logError("Unknown command.");
        }

    }
    else if(!strcmp(cmdPart1,"order")&&!strcmp(cmdPart2,"food")){
        ordering();
        logMsg(csr.username,"Ordering finished successfully.");
    }
    else{
        logError("Unknown command.");
    }
}

void interface(){
    char msgBuf[MAX_BUFF];
    char brodcBuf[MAX_BUFF]={0};
    FdSet fdSet;
    fdSet.maxFd=0;
    FD_ZERO(&fdSet.masterSet);
    FD_SETTER(STDIN_FILENO, &fdSet);
    FD_SETTER(csr.bcast.bcastSocket, &fdSet);
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
            else if(i==csr.bcast.bcastSocket){
                memset(brodcBuf, 0, 1024);
                recv(i,brodcBuf, 1024, 0);
                logMsg(csr.username,"Recive something on broadcast.");
                brodCastAction(brodcBuf);
                logMsg(csr.username,"Action on brodcast successfully.");
            }
            else{
                logWarning("Unknown file descriptor.");
            }
        }
    }
}

void initBrodcast(const char* port){
  csr.udpPort= strToPort(port);
  csr.bcast.bcastSocket = makeBroadcast(BRODCAST_IP,csr.udpPort, &csr.bcast.addr);
  logInfo("Broadcast receiving successfully initialized.");
}

void initServer(){
    while (1)
    {
        csr.tcpPort=generateRandomPort();
        csr.serverSocket=makeServer(csr.tcpPort);
        if(csr.serverSocket>=0)
            break;
    }
    logInfo("Making server succesfully.");
}

void checkDuplicate(){
    signal(SIGALRM,alarmhandler);
    siginterrupt(SIGALRM,1);
    char responseBuf[BCAST_MSG];
    char brodMsg[BCAST_MSG];
    sprintf(brodMsg,"username %s",csr.username);
    sendBcast(csr.bcast.addr,csr.bcast.bcastSocket,brodMsg);
    while(1){
        int res=-1;
        memset(responseBuf, 0, BCAST_MSG);
        alarm(1);
        res=recv(csr.bcast.bcastSocket,responseBuf,BCAST_MSG,0);
        alarm(0);
        if(res<0)  return;
        if(!strcmp(responseBuf,DUP_ERROR)){
            logError("Duplicate username!");
            getInput(STDIN_FILENO, "Enter your username: ", csr.username, NAME_SIZE);
            sprintf(brodMsg,"username %s",csr.username);
            sendBcast(csr.bcast.addr,csr.bcast.bcastSocket,brodMsg);
        }
    } 
}

void initUsername(){
    getInput(STDIN_FILENO, "Please enter your username: ", csr.username, NAME_SIZE);
    checkDuplicate();
}


void initCustomer(const char* port){
  initBrodcast(port);
  initServer();
  initUsername();
  createLogFile(csr.username);
  logMsg(csr.username,"Customer created successfully."); 
  char welcomeMsg[MAX_MSG];
  sprintf(welcomeMsg,"welcome %s %s %s as customer\n",ANSI_GRN,csr.username,ANSI_RST);
  write(STDOUT_FILENO,welcomeMsg,strlen(welcomeMsg));
}


int main(int argc, const char* argv[]){
  if (argc != 2) {
    logError("Wrong number of arguments.");
    return EXIT_FAILURE;
  }
  initCustomer(argv[1]);
  interface();
}