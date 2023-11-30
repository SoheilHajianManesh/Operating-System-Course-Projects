#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <netdb.h>

#include "loger.h"
#include "defs.h"
#include "ansi-color.h"

void createLogFile(char* name)
{
    char path[MAX_BUFF];
    sprintf(path, "%s.log", name);
    int fd = creat( path, RD_WR );
    if(fd == -1) {
        logError("Error creating log file!");    
        return;
    }
    logInfo("create log file successfully!");
    close(fd);
}

void logMsg(char* name, char* msg)
{
    char path[MAX_BUFF];
    sprintf(path, "%s.log", name);
    int fd = open(path, O_WRONLY | O_APPEND);
    char buf[MAX_BUFF];
    sprintf(buf, "%s\n", msg);
    write(fd, buf, strlen(buf));
    close(fd);
}

void logError(char *errorMsg){
    write(STDERR_FILENO,ANSI_RED "[Error] " ANSI_RST,9+ANSI_LEN);
    write(STDERR_FILENO,errorMsg,strlen(errorMsg));
    write(STDOUT_FILENO, "\n", 1);
}

void logInfo(char* msg){
    write(STDOUT_FILENO, ANSI_BLU "[Info] " ANSI_RST, 7 + ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, "\n", 1);
}

void logWarning(char* msg){
    write(STDOUT_FILENO, ANSI_YEL "[Warning] " ANSI_RST, 10 + ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, "\n", 1);
}
