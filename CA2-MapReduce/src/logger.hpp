#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <string>

class Logger{

    public:
        Logger(std::string proccessName);
        void logError(const std::string &errorMsg);
        void logInfo(const std::string &infoMsg);
        void logWarning(const std::string &warningMsg);

    private:
        std::string proccessName;

};

#endif