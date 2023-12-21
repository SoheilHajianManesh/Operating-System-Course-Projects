#include <iostream>
#include <chrono>
#include <pthread.h>

#include "include/bmp.hpp"
#include "include/filter.hpp"
#include "include/thread.hpp"


using namespace std;

using TimeElapsed = chrono::duration<float,milli>;

ThreadData* args;

TimeElapsed::rep readPic(Bmp24 &img,int threadCount){
    auto tStart=chrono::high_resolution_clock::now();
    creadteAndJoinThreads(img.getPixelsFromBMP24,threadCount,args);
    auto tEnd=chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeElapsed>(tEnd - tStart).count();
}

TimeElapsed::rep flipVertical(Bmp24 &img,int threadCount){
    auto tStart=chrono::high_resolution_clock::now();
    creadteAndJoinThreads(filter::flipVertical,threadCount,args);
    auto tEnd=chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeElapsed>(tEnd - tStart).count();
}

TimeElapsed::rep blur(Bmp24 &img,int threadCount){
    auto tStart=chrono::high_resolution_clock::now();
    creadteAndJoinThreads(filter::blur,threadCount,args);
    auto tEnd=chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeElapsed>(tEnd - tStart).count();
}

TimeElapsed::rep purpleHaze(Bmp24 &img,int threadCount){
    auto tStart=chrono::high_resolution_clock::now();
    creadteAndJoinThreads(filter::purpleHaze,threadCount,args);
    auto tEnd=chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeElapsed>(tEnd - tStart).count();
}

TimeElapsed::rep origonalHatch(Bmp24 &img,int threadCount){
    auto tStart=chrono::high_resolution_clock::now();
    creadteAndJoinThreads(filter::origonalHatch,threadCount,args);
    auto tEnd=chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeElapsed>(tEnd - tStart).count();
}


int main(int argc, char* argv[]){
    if(argc != 2){
        cerr<<"Wrong Input Format"<<endl;
        return EXIT_FAILURE;
    }
    auto startExec=chrono::high_resolution_clock::now();
    Bmp24 img(argv[1]);
    if (!img.fillAndAllocate())
    {
        cout << "File read error" << endl;
        return 1;
    }

    int threadCount =THREAD_COUNT;
    args= createArgs(img,threadCount);
    auto readTime = readPic(img,threadCount);
    auto flipTime=flipVertical(img,threadCount);
    auto blurTime=blur(img,threadCount);
    auto purpleTime=purpleHaze(img,threadCount); 
    auto linesTime=origonalHatch(img,threadCount);

    creadteAndJoinThreads(img.writeOutBmp24,threadCount,args);
    auto endExec=chrono::high_resolution_clock::now();

    cout<<"readTime: "<< readTime <<" ms\n";
    cout<<"Flip: "<< flipTime <<" ms\n";
    cout<<"Blur: "<< blurTime <<" ms\n";
    cout<<"Purple: "<< purpleTime <<" ms\n";
    cout<<"Lines: "<< linesTime <<" ms\n";
    cout<<"Execution: "<< chrono::duration_cast<TimeElapsed>(endExec - startExec).count()<<" ms\n";
}