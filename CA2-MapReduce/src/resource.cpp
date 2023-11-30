#include <unistd.h>
#include <iostream>
#include <vector>
#include <numeric> 
#include <algorithm>

#include "logger.hpp"
#include "consts.hpp"
#include "strops.hpp"
#include "csv.hpp"


using namespace std;

Logger log("resource");


string calcDatas(vector<vector<string>> table,vector<string> monthes){
    vector<string> month(monthes.size());
    vector<int> averageConsume(monthes.size());
    vector<int> totalConsume(monthes.size());
    vector<int> busyHour(monthes.size());
    vector<int> lowHoursConsume(monthes.size());
    vector<int> avgBusyDiff(monthes.size());
    string result="";
    for(int i=0;i<monthes.size();i++){
        vector<int>hours(HOURS,0);
        for(int j=(stoi(monthes[i])-1)*MONTH_DAYS+1;j<=stoi(monthes[i])*MONTH_DAYS;j++){
            for(int hour=0;hour<HOURS;hour++)
                hours[hour]+=stoi(table[j][hour+3]);
        }
        totalConsume[i]=accumulate(hours.begin(),hours.end(),0);  
        averageConsume[i]=totalConsume[i]/MONTH_DAYS;
        busyHour[i]=distance(hours.begin(),max_element(hours.begin(),hours.end()));
        avgBusyDiff[i]=(hours[busyHour[i]]*HOURS/MONTH_DAYS)-averageConsume[i];
        lowHoursConsume[i]=0;
        for(int k=0;k<hours.size();k++){
            if(hours[k]*HOURS/MONTH_DAYS<averageConsume[i])
                lowHoursConsume[i]+=hours[k];
        }
        result+=monthes[i]+" "+to_string(averageConsume[i])+" "+to_string(totalConsume[i])+" "
            +to_string(busyHour[i])+ " " +to_string(avgBusyDiff[i])+ " " + to_string(hours[busyHour[i]]) 
                + " " +to_string(lowHoursConsume[i])+"$"+"\0";
    }
    return result;
    
}

int main(int argc, char const *argv[])
{
     if(argc!=3){
        log.logError("Incorrrent number of arguments");
        return EXIT_FAILURE;
    }

    int readFromBuildingFd=atoi(argv[1]);
    int writeToBuildingFd=atoi(argv[2]);
    
    //get information from building proccess
    char buffer_[MAX_BUF];
    int bytesRead = read(readFromBuildingFd, buffer_, MAX_BUF);
    buffer_[bytesRead] = '\0';
    string buffer=string(buffer_);
    close(readFromBuildingFd);
    string path=strops::split(buffer,DELIMETER)[0];
    string monthes_=strops::split(buffer,DELIMETER)[1];
    vector<string> monthes=strops::split(monthes_,SPACE_SEPARETOR);
    log.logInfo("Information recived successfully.");
    

    //read csv file
    Csv csv(path+".csv");
    csv.readCsv();
    auto table=csv.getTable();
    string result=calcDatas(table,monthes);
    log.logInfo("Data calculated successfully.");
    write(writeToBuildingFd,result.c_str(),result.size());
    close(writeToBuildingFd);
    log.logInfo(path+" proccessed exit successfully.");
    exit(EXIT_SUCCESS);
}
