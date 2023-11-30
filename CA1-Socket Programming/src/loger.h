#ifndef LOGER_H
#define LOGER_H

#define RD_WR 0666

void createLogFile(char* name);
void logMsg(char* name, char* msg);
void logError(char* errorMsg);
void logInfo(char* msg);
void logWarning(char* msg);

#endif