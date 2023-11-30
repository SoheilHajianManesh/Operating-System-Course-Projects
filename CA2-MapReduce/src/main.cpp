#include <unistd.h>
#include <sys/wait.h>

#include <iostream>
#include <vector>
#include <filesystem>
#include <algorithm>

#include "logger.hpp"
#include "ansi-color.hpp"
#include "strops.hpp"
#include "consts.hpp"
#include "utils.hpp"

using namespace std;

namespace fs= filesystem;
Logger log("main");

void showFinalResult(string result,string building){
    vector<string> resultParts=strops::split(result,'&');
    string billResult=resultParts[0];
    string buildingResult=resultParts[1];
    vector<string> billResultParts=strops::split(billResult,RESOURCE_DELIMITER);
    vector<string> buildingResultParts=strops::split(buildingResult,RESOURCE_DELIMITER);
    cout<<"Building " << ANSI_YEL <<building<< ANSI_RST <<endl;
    for (int i = 0; i <buildingResultParts.size(); i++)
    {
        vector<string> billResourceparts=strops::split(billResultParts[i],DELIMETER);
        vector<string> buildingResourceparts=strops::split(buildingResultParts[i],DELIMETER);
        cout<<"\t"<<ANSI_CYN<<billResourceparts[0]<<":"<<ANSI_RST<<endl;
        for(int j = 1; j < buildingResourceparts.size(); j++){
            vector<string> billMonthParts=strops::split(billResourceparts[j],SPACE_SEPARETOR);
            vector<string> buildingMonthParts=strops::split(buildingResourceparts[j],SPACE_SEPARETOR);
            cout<<"\t\t"<<ANSI_GRN<<"Month "<<buildingMonthParts[0]<<" :"<<ANSI_RST<<endl;
            cout<<"\t\t\t"<<ANSI_WHT<<"Average consumption : "<<ANSI_BLU<<buildingMonthParts[1]<<ANSI_RST<<endl;
            cout<<"\t\t\t"<<ANSI_WHT<<"Total consumption : "<<ANSI_BLU<<buildingMonthParts[2]<<ANSI_RST<<endl;
            cout<<"\t\t\t"<<ANSI_WHT<<"Busy hour : "<<ANSI_BLU<<buildingMonthParts[3]<<ANSI_RST<<endl;
            cout<<"\t\t\t"<<ANSI_WHT<<"Bill : "<<ANSI_BLU<<billMonthParts[1]<<ANSI_RST<<endl;
            cout<<"\t\t\t"<<ANSI_WHT<<"Difference between average consumption and consumption in busy hour : "
                <<ANSI_BLU<<buildingMonthParts[4]<<ANSI_RST<<endl;
        }
    }

}

void showBuildings(vector<fs::path> &buildings){
    int k=1;
    log.logInfo("We have " + to_string(buildings.size()) + " buildings:");
    for(auto& building: buildings) {
        vector<string> buildingsEntries=strops::split(building,SLASH);
        log.logInfo(ANSI_GRN +to_string(k) + "- " + ANSI_WHT 
                        + buildingsEntries[1] + ANSI_RST);
        k++;
    }
}

void getWantedBuildings(vector<string>& wantedBuildings){
    string buffer; 
    cout << ANSI_MAG << "Which building(s) you want:"<<ANSI_YEL<<"(Names of building(s) - space separated - in order)"<<ANSI_RST<<endl;
    getline(cin,buffer);
    wantedBuildings=strops::split(buffer,SPACE_SEPARETOR);  
}

void getWantedResources(vector<string>& wantedBuildings,vector<string>& wantedResources){
    string buffer;
    for(int i=0;i<wantedBuildings.size();i++){
        cout << ANSI_GRN <<  wantedBuildings[i] << " : " << ANSI_MAG << 
            "resource(s) (Water/Gas/Electricity)"<<ANSI_YEL<<
                "(Names of resource(s) - space separated)"<<ANSI_RST<<endl;
        getline(cin,buffer);
        wantedResources[i]=buffer;
    }
}

void getWantedMonth(vector<string>& wantedBuildings,vector<string>& wantedMonthes){
    for(int i=0;i<wantedBuildings.size();i++){
        string buffer;
        cout << ANSI_GRN <<  wantedBuildings[i] << " : " << ANSI_MAG << 
            "Which month(s) you want"<<ANSI_YEL<<
                "(Number of month - space seperate)"<<ANSI_RST<<endl;
        getline(cin,buffer);
        wantedMonthes[i]=buffer;
    }
}
int createNamedPipes(int numOfBuildings,vector<vector<int>>&buildingsToMainPipes,vector<vector<int>> mainToBuildingsPipes){
    for(int i = 0; i<numOfBuildings;i++){
        if(pipe(buildingsToMainPipes[i].data()) == -1){
            log.logError("Cannot create pipe for building " + to_string(i+1));
            return EXIT_FAILURE;
        }
        if(pipe(mainToBuildingsPipes[i].data()) == -1){
            log.logError("Cannot create pipe for building " + to_string(i+1));
            return EXIT_FAILURE;
        }
    }
}
int createBuildingsProcesses(vector<vector<int>> &buildingsToMainPipes,vector<vector<int>> &mainToBuildingsPipes,vector<fs::path> &buildings)
{
    for(int i = 0; i<buildings.size();i++){
        int pid=fork();
        if(pid<0){
            log.logError("Cannot create child process for building " + strops::split(buildings[i],SLASH)[1]);
            return EXIT_FAILURE;
        }
        else if(pid==0){//child proccess
            char path[MAX_BUF];
            char readFd[FD_MAX];
            char writeFd[FD_MAX];
            sprintf(path,"%s",buildings[i].string().c_str());
            close(mainToBuildingsPipes[i][1]);
            close(buildingsToMainPipes[i][0]);
            sprintf(readFd,"%d",mainToBuildingsPipes[i][0]);           
            sprintf(writeFd,"%d",buildingsToMainPipes[i][1]);
            if(execl(OUT_BUILDING,OUT_BUILDING,path,readFd,writeFd,NULL)==-1){
                log.logError("Cannot execute .out file for building " + strops::split(buildings[i],SLASH)[1]);
                return EXIT_FAILURE;
            }
        }
        else if(pid>0){//parent proccess
            close(buildingsToMainPipes[i][1]);
            close(mainToBuildingsPipes[i][0]);
        }
    }
}

int createBillProccess(){
    int pid=fork();
    if(pid<0){
        log.logError("Cannot create child process for bill");
        return EXIT_FAILURE;
    }
    else if(pid==0){//child proccess
        if(execl(OUT_BILL,OUT_BILL,NULL)==-1){
            log.logError("Cannot execute .out file for bill.");
            return EXIT_FAILURE;
        }
    }
}

int main(int argc, const char* argv[]){
    if(argc != 2){
        cerr << ANSI_RED << "correct format: "
                  << ANSI_GRN << "BuildingsBillStats.out" 
                  << ANSI_MAG << " <building folder>\n" << ANSI_RST;
        return EXIT_FAILURE;
    }
    vector<fs::path> buildings;
    getDirFolders(argv[1], buildings,log);
    showBuildings(buildings);

    //create unnamed pipes between buildings and main proccess
    vector<vector<int>> buildingsToMainPipes(buildings.size(), vector<int>(2));
    vector<vector<int>> mainToBuildingsPipes(buildings.size(), vector<int>(2));
    for(int i = 0; i<buildings.size();i++){
        if(pipe(buildingsToMainPipes[i].data()) == -1){
            log.logError("Cannot create pipe for building " + to_string(i+1));
            return EXIT_FAILURE;
        }
        if(pipe(mainToBuildingsPipes[i].data()) == -1){
            log.logError("Cannot create pipe for building " + to_string(i+1));
            return EXIT_FAILURE;
        }
    }
    log.logInfo("Pipes between buildings and main created successfully.");

    //create buildings proccesses
    createBuildingsProcesses(buildingsToMainPipes,mainToBuildingsPipes,buildings);

    log.logInfo("Buildings proccesses created successfully.");


    //get information from user
    vector<string> wantedBuildings;
    getWantedBuildings(wantedBuildings);
    vector<string> wantedResources(wantedBuildings.size());
    vector<string> wantedMonthes(wantedBuildings.size());
    getWantedResources(wantedBuildings,wantedResources);
    getWantedMonth(wantedBuildings,wantedMonthes);
    log.logInfo("Information received successfully.");

    //send Informations to buildings
    for(int i=0;i<wantedBuildings.size();i++){
        string information = wantedResources[i] + "$" + wantedMonthes[i] + "\0";
        auto it=find(buildings.begin(),buildings.end(),"buildings/"+wantedBuildings[i]);
        if(it!=buildings.end()){
            write(mainToBuildingsPipes[it-buildings.begin()][1],information.c_str(),information.size());
        }
        else{
            log.logError("Building "+wantedBuildings[i]+" not found.");
        }
    }
    log.logInfo("Information sent to buildings successfully.");

    sleep(2);
    //create bill proccess
    createBillProccess();
    log.logInfo("Bill proccess created successfully.");
    //wait for buildings proccesses
    for (int i = 0; i < wantedBuildings.size(); i++) {
        int status;
        wait(&status);
    }

    log.logInfo("All children proccesses exit successfully.");
    //get results from buildings
    for(int i=0;i<wantedBuildings.size();i++){
        char finalResult[FINAL_BUFFER];
        auto it=find(buildings.begin(),buildings.end(),"buildings/"+wantedBuildings[i]);
        if(it!=buildings.end()){
            int bytesRead=read(buildingsToMainPipes[it-buildings.begin()][0],finalResult,FINAL_BUFFER);
            finalResult[bytesRead] = '\0';
        }
        else{
            log.logError("Building "+wantedBuildings[i]+" not found.");
        }
        showFinalResult(finalResult,wantedBuildings[i]);
    }
    log.logInfo("Project Finished Successfully.");
    exit(EXIT_SUCCESS);
}