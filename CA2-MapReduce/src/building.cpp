#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <vector>
#include <string>
#include <filesystem>
#include <string.h>
#include <iostream>

#include "consts.hpp"
#include "logger.hpp"
#include "strops.hpp"
#include "utils.hpp"

using namespace std;

namespace fs=filesystem;
Logger log("building");

string getResourcesResult(int numOfResources,vector<string>wantedResources,vector<vector<int>> resourceToBuildingPipes){
    char resourcesResults_[numOfResources][MAX_BUF];
    vector<string> resourcesResults(numOfResources);
    //read from pipes
    for (int i = 0; i < numOfResources; i++) {
        int bytesRead=read(resourceToBuildingPipes[i][0],resourcesResults_[i],MAX_BUF);
        resourcesResults_[i][bytesRead]='\0';
        resourcesResults[i]=string(resourcesResults_[i]);
    }

    string codedResult="";
    for(int i = 0; i < numOfResources; i++) {
        codedResult+=wantedResources[i]+"$"+resourcesResults[i]+RESOURCE_DELIMITER;
    }
    return codedResult;
}

string getBillResults(string path,string codedResult){
    //make namedPipes
    string pipeName = string(PIPE_PATH) + "/" + strops::split(path,SLASH)[1];
    if (mkfifo(pipeName.c_str(), 0666) == -1) {
        log.logError("Can't create named pipe");
        return NULL;
    }

    sleep(1.5);
    //open pipe for writing
    int writeFd = open(pipeName.c_str(), O_WRONLY);
    if (writeFd == -1) {
        log.logError("Can't open pipe for writing: " + pipeName);
        return NULL;
    }
    write(writeFd, codedResult.c_str(),codedResult.size());
    close(writeFd);

    //open pipe for reading
    int readFd = open(pipeName.c_str(), O_RDONLY);
    if (readFd == -1) {
        log.logError("Can't open pipe reading: " + pipeName);
        return NULL;
    }
    char billResult[MAX_BUF];
    int bytesRead=read(readFd, billResult, MAX_BUF);
    billResult[bytesRead] = '\0';
    close(readFd);
    return string(billResult);
}

int main(int argc, char* argv[]){
    if(argc!=4){
        log.logError("Incorrrent number of arguments");
        return EXIT_FAILURE;
    }
    int readFromMainFd=atoi(argv[2]);
    int writeToMainFd=atoi(argv[3]);

    //get information from main proccess
    char buffer_[MAX_BUF];
    int bytesRead = read(readFromMainFd, buffer_, MAX_BUF);
    buffer_[bytesRead] = '\0';

    string buffer=string(buffer_);
    close(readFromMainFd);
    string resources=strops::split(buffer,DELIMETER)[0];
    vector<string> wantedResources=strops::split(resources,SPACE_SEPARETOR);
    string wantedmMonthes=strops::split(buffer,DELIMETER)[1];
    log.logInfo("Information recive successfully");
    
    
    //create unnamed pipes between building and resources
    vector<vector<int>> buildingToResourcePipes(wantedResources.size(), vector<int>(2));
    vector<vector<int>> resourceToBuildingPipes(wantedResources.size(), vector<int>(2));
    for(int i=0; i<wantedResources.size(); i++){
        if(pipe(buildingToResourcePipes[i].data())==-1){
            log.logError("Cannot create pipe for resource " + wantedResources[i]);
            return EXIT_FAILURE;
        }
        if(pipe(resourceToBuildingPipes[i].data())==-1){
            log.logError("Cannot create pipe for resource " + wantedResources[i]);
            return EXIT_FAILURE;
        }
    }

    // create resources proccesses
    for(int i = 0; i<wantedResources.size();i++){
        int pid=fork();
        if(pid<0){
            log.logError("Cannot create child process for resource " + wantedResources[i]);
            return EXIT_FAILURE;
        }
        else if(pid==0){//child proccess
            char readFd[FD_MAX];
            char writeFd[FD_MAX];
            close(buildingToResourcePipes[i][1]);
            close(resourceToBuildingPipes[i][0]);
            sprintf(readFd,"%d",buildingToResourcePipes[i][0]);
            sprintf(writeFd,"%d",resourceToBuildingPipes[i][1]);
            if(execl(OUT_RESOURCE,OUT_RESOURCE,readFd,writeFd,NULL)==-1){
                string a=argv[1];
                log.logError(a +" Cannot execute .out file for resource " + wantedResources[i]);
                return EXIT_FAILURE;
            }
        }
        else if(pid>0){//parent proccess
            string path(argv[1]);
            string information= path + "/" + wantedResources[i] + "$" + wantedmMonthes + "\0";
            close(buildingToResourcePipes[i][0]);
            close(resourceToBuildingPipes[i][1]);
            write(buildingToResourcePipes[i][1],information.c_str(),information.size());
            close(buildingToResourcePipes[i][1]);
        }
    }

    //wait for resources procceesses to finish
    for (int i = 0; i < wantedResources.size(); i++) {
        int status;
        wait(&status);
    }

    //get result of resources
    string codedResult=getResourcesResult(wantedResources.size(),wantedResources,resourceToBuildingPipes);
    //get result of bill
    string billResult=getBillResults(argv[1],codedResult);
    string finalResult = billResult + RESULT_PART_SEPARATOR + codedResult;
    write(writeToMainFd,finalResult.c_str(),finalResult.size());
    log.logInfo("Send result to main successfully.");
    close(writeToMainFd);
    log.logInfo("Building process finished successfully.");
    exit(EXIT_SUCCESS);
}