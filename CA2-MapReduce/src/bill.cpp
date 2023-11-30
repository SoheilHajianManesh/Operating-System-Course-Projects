#include <unistd.h>
#include <fcntl.h>

#include <filesystem>
#include <iostream>
#include <algorithm>

#include "logger.hpp"
#include "utils.hpp"
#include "consts.hpp"
#include "strops.hpp"
#include "csv.hpp"



using namespace std;

namespace fs= filesystem;

Logger log("bill");


string calcBill(string results,vector<vector<string> >& table,int numberOfMonths){    
    vector<string> resultsPerMonth=strops::split(results,DELIMETER);
    string response="";
    for(int i=1;i<numberOfMonths;i++){
        vector<string> tokens=strops::split(resultsPerMonth[i],SPACE_SEPARETOR);
        int bill;
        int month=stoi(tokens[0]);
        int totalConsump=stoi(tokens[2]);
        int busyHourConsump=stoi(tokens[5]);
        int lowHourConsump=stoi(tokens[6]);
        if(resultsPerMonth[0]==WATER){
            totalConsump-=busyHourConsump;
            bill=totalConsump*stoi(table[month][2]);
            bill+=busyHourConsump*stoi(table[month][2])*1.25;
        }
        else if (resultsPerMonth[0]==GAS)
        {
            bill=totalConsump*stoi(table[month][3]);
        }
        else if(resultsPerMonth[0]==ELECTRICITY){
            totalConsump-=busyHourConsump;
            totalConsump-=lowHourConsump;
            bill=totalConsump*stoi(table[month][4]);
            bill+=busyHourConsump*stoi(table[month][4])*1.25;
            bill+=lowHourConsump*stoi(table[month][4])*0.75;
        }
        response+=to_string(month) + SPACE_SEPARETOR + to_string(bill)+ DELIMETER+"\0";
    }
    return resultsPerMonth[0]+DELIMETER+response;
}

int main(int argc, char* argv[]){
    //read Csv file
    Csv csv("buildings/bills.csv");
    csv.readCsv();
    auto table=csv.getTable();   
    //get pipes files 
    vector<fs::path> pipes;
    getDirFiles(PIPE_PATH, pipes,log);
    for (int i = 0; i < pipes.size(); i++)
    {
        int readFd = open(pipes[i].c_str(), O_RDONLY);
        if (readFd== -1) {
            log.logError("Can't open pipe for reading: " + pipes[i].string());
                return EXIT_FAILURE;
        }
        log.logInfo("Open Pipe " + pipes[i].string()+" successfully.");
        char buffer_[MAX_BUF];
        int bytesRead = read(readFd, buffer_, MAX_BUF);
        buffer_[bytesRead] = '\0';
        string buffer=string(buffer_);
        close(readFd);
        vector<string> resourcesResults=strops::split(buffer,RESOURCE_DELIMITER);
        int numberOfMonth=strops::split(resourcesResults[0],DELIMETER).size();
        string incodeResponse="";
        for (int j = 0; j < resourcesResults.size();j++){
            incodeResponse+=calcBill(resourcesResults[j],table,numberOfMonth)+RESOURCE_DELIMITER;
        }
        log.logInfo("Bill Calc Successfully");
        int writeFd = open(pipes[i].c_str(), O_WRONLY);
        if (writeFd == -1) {
            log.logError("Can't open pipe for writing: " + pipes[i].string());
                return EXIT_FAILURE;
        }
        write(writeFd, incodeResponse.c_str(), incodeResponse.size());
        log.logInfo("Send Bill to building successfully.");
        close(writeFd);
    }
    log.logInfo("Bill proccess finished successfully.");
    exit(EXIT_SUCCESS);
}