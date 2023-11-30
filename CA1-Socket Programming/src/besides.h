#ifndef BESIDES_H_INCLUDE
#define BESIDES_H_INCLUDE

#include "defs.h"

void cliPrimary();
int strToPort(const char* str);
void getInput(int fd, char *request, char *buffer, int bufferSize);
int generateRandomPort();
char* tokenizeStr(char *str, const char *delimiters);
void alarmhandler(int sig);
#endif