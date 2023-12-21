#include <iostream>
#include <chrono>

#include "include/bmp.hpp"
#include "include/filter.hpp"

using namespace std;

using TimeElapsed = chrono::duration<float,milli>;

TimeElapsed::rep flipVertical(Bmp24 &img){
    auto tStart=chrono::high_resolution_clock::now();
    filter::flipVertical(img);
    auto tEnd=chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeElapsed>(tEnd - tStart).count();
}

TimeElapsed::rep blur(Bmp24 &img){
    auto tStart=chrono::high_resolution_clock::now();
    filter::blur(img);
    auto tEnd=chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeElapsed>(tEnd - tStart).count();
}

TimeElapsed::rep purpleHaze(Bmp24 &img){
    auto tStart=chrono::high_resolution_clock::now();
    filter::purpleHaze(img);
    auto tEnd=chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeElapsed>(tEnd - tStart).count();
}

TimeElapsed::rep origonalHatch(Bmp24 &img){
    auto tStart=chrono::high_resolution_clock::now();
    filter::origonalHatch(img);
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
    auto startRead=chrono::high_resolution_clock::now();
    img.getPixelsFromBMP24();
    auto endRead=chrono::high_resolution_clock::now();
    auto flipTime=flipVertical(img);
    auto blurTime=blur(img);
    auto pupleTime=purpleHaze(img); 
    auto linesTime=origonalHatch(img);
    img.writeOutBmp24("output.bmp");
    auto endExec=chrono::high_resolution_clock::now();

    cout<<"Read: "<< chrono::duration_cast<TimeElapsed>(endRead - startRead).count()<<" ms\n";
    cout<<"Flip: "<< flipTime <<" ms\n";
    cout<<"Blur: "<< blurTime <<" ms\n";
    cout<<"Purple: "<< pupleTime <<" ms\n";
    cout<<"Lines: "<< linesTime <<" ms\n";
    cout<<"Execution: "<< chrono::duration_cast<TimeElapsed>(endExec - startExec).count()<<" ms\n";

}