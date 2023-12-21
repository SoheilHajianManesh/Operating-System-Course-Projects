#ifndef THREAD_HPP
#define THREAD_HPP

#include <pthread.h>
#include <iostream>

#include "utils.hpp"
#include "bmp.hpp"


ThreadData* createArgs(Bmp24 &img,int threadCount){
    ThreadData* args= new ThreadData[THREAD_COUNT];
    int cols=img.getWidth();
    int rows=img.getHeight();
    int extra=cols%4;
    int rowPartitionSize = rows/threadCount;
    int colPartitionSize = cols/threadCount;
    for(int i=0;i<threadCount;i++){
        if(i==threadCount-1){
            args[i].rowEnd = rows;
            args[i].colEnd = cols;
        }
        else {
            args[i].rowEnd = (i+1)*rowPartitionSize;
            args[i].colEnd=(i+1)*colPartitionSize;
        }
        args[i].rowStart = i*rowPartitionSize;
        args[i].colStart=i*colPartitionSize;
        args[i].rows=rows;
        args[i].cols=cols;
        args[i].index =(i *rowPartitionSize) * (cols * 3 + extra) + 1;
        args[i].fileName="output.bmp";
        args[i].end=img.getEnd();
        args[i].buffer =img.getBuffer();
        args[i].image = img.getImage();
    }
    return args; 
}
void creadteAndJoinThreads(void* (*func)(void*) , int numberOfThreads,ThreadData* args)
{
  pthread_t threads[numberOfThreads];
  int rc;
  long t;
  for (t = 0; t < numberOfThreads; t++)
  {
    rc = pthread_create(&threads[t], NULL, func, &args[t]);
    if (rc)
    {
      std::cout << "Error:unable to create thread" << rc << std::endl;
      exit(-1);
    }
  }
  for (t = 0; t < numberOfThreads; t++)
  {
    rc = pthread_join(threads[t], NULL);
    if (rc)
    {
      std::cout << "Error:unable to join threads" << rc << std::endl;
      exit(-1);
    }
  }
}
#endif