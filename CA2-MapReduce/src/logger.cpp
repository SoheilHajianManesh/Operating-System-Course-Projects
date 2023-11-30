#include <iostream>
#include <cstring>
#include <cerrno>

#include "logger.hpp"
#include "ansi-color.hpp"

using namespace std;

Logger::Logger(string proccessName):proccessName(proccessName)
{
}

void Logger::logError(const string &errorMsg){
    cerr << ANSI_RED << "[ERROR: " << proccessName << "] "<< ANSI_RST << errorMsg<< '\n';
}

void Logger::logInfo(const string &infoMsg){
    cout << ANSI_BLU << "[Info: " << proccessName << "] "<< ANSI_RST << infoMsg << '\n';
}

void Logger::logWarning(const string &warningMsg){
    cout << ANSI_YEL << "[WARNING: " << proccessName << "] "<< ANSI_RST << warningMsg << '\n';
}
