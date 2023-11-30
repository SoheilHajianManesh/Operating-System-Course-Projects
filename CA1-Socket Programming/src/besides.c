#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "loger.h"
#include "besides.h"
#include "ansi-color.h"


void cliPrimary() {
    write(STDOUT_FILENO,ANSI_WHT ">> " ANSI_RST, 3+ANSI_LEN);
}

int strToPort(const char* str){
    int res=0;
    if (str[0] == '\0') {
        logError("Empty port.");
        exit(EXIT_FAILURE);
    }
    while (*str != '\0') {
        if(!isdigit(*str)) {
            logError("Invalid port.");
            exit(EXIT_FAILURE);
        }
        res = res * 10 + (*str - '0');
        str++;
    }
    if(res > MAX_PORT){
        logError("Invalid port.");
        exit(EXIT_FAILURE);
    }
    return res;
}

void getInput(int fd, char *request, char *buffer, int bufferSize) {
    if(request!=NULL){
        write(fd, request, strlen(request));
    }
    int bytesRead = read(fd, buffer, bufferSize);
    if (bytesRead > 0) {
        // Remove the trailing newline character from the input
        if (buffer[bytesRead - 1] == '\n') {
            buffer[bytesRead - 1] = '\0';
        }
    }
}

int generateRandomPort(){
    srand(time(NULL));
    int randomPort = rand() % MAX_PORT;    
    return randomPort;
}

char* tokenizeStr(char* str, const char* delimiter) {
    char* token = strtok(str, delimiter);
    if (token != NULL) {
        int tokenLength = strlen(token);
        char* deletedPart = (char*)malloc(tokenLength + 1);
        strcpy(deletedPart, token);
        strcpy(str, str + tokenLength + strlen(delimiter));  // Modify the original string
        return deletedPart;
    } else {
        return NULL;
    }
}

void alarmhandler(int sig){
}





