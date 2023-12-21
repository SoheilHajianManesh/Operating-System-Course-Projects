#include <fstream>
#include <iostream>
#include <string.h>
#include "include/bmp.hpp"

using namespace std;

Bmp24::Bmp24(char *fileName):fileName(fileName)
{
}

bool Bmp24::fillAndAllocate() {
    ifstream file(fileName);
    if (!file) {
        cerr << "File " << fileName << " doesn't exist!" << endl;
        return false;
    }

    file.seekg(0, ios::end);
    streampos length = file.tellg();
    file.seekg(0, ios::beg);

    fileBuffer = new char[length];
    file.read(&fileBuffer[0], length);

    file_header = (PBITMAPFILEHEADER)(&fileBuffer[0]);
    info_header = (PBITMAPINFOHEADER)(&fileBuffer[0] + sizeof(BITMAPFILEHEADER));
    rows = info_header->biHeight;
    cols = info_header->biWidth;
    bufferSize = file_header->bfSize;
    image.resize(rows);
    for(int i = 0; i < rows; i++){
        image[i].resize(cols);
    }
    return true;
}

void* Bmp24::getPixelsFromBMP24(void* arguments) {
    ThreadData* args=(ThreadData*) arguments;
    int rowStart = args->rowStart;
    int rowEnd = args->rowEnd;
    int idx = args->index;
    int end=args->end;
    int extra = args->cols % 4;
    int cols_=args->cols;
    char *buffer=args->buffer;
    Image* image=args->image;
    for (int i = rowStart; i < rowEnd; i++) {
        idx+=extra;
        for (int j = cols_ - 1; j >= 0; j--) {
            for (int k = 0; k < 3; k++) {
                switch (k) {
                case 0:
                    (*image)[i][j].red=static_cast<unsigned int>(static_cast<unsigned char>(buffer[end-idx]));
                    break;
                case 1:
                    (*image)[i][j].green=static_cast<unsigned int>(static_cast<unsigned char>(buffer[end-idx]));
                    break;
                case 2:
                    (*image)[i][j].blue=static_cast<unsigned int>(static_cast<unsigned char>(buffer[end-idx]));
                    break;
                }
                idx++;
            }
        }
    }
    return nullptr;
}

void* Bmp24::writeOutBmp24(void* arguments) {
    ThreadData* args=(ThreadData*) arguments;
    int rowStart = args->rowStart;
    int rowEnd = args->rowEnd;
    int idx = args->index;
    int end=args->end;
    int extra = args->cols % 4;
    int cols_=args->cols;
    char *buffer=args->buffer;
    Image* image=args->image;
    const char* nameOfFileToCreate=args->fileName;

    std::ofstream write(nameOfFileToCreate);
    if (!write) {
        std::cout << "Failed to write " << nameOfFileToCreate << std::endl;
        return nullptr;
    }

    for (int i = rowStart; i < rowEnd; i++) {
        idx += extra;
        for (int j = cols_ - 1; j >= 0; j--) {
            for (int k = 0; k < 3; k++) {
                switch (k) {
                case 0:
                    buffer[end-idx]=char((*image)[i][j].red);
                    break;
                case 1:
                    buffer[end-idx]=char((*image)[i][j].green);
                    break;
                case 2:
                    buffer[end-idx]=char((*image)[i][j].blue);
                    break;
                }
                idx++;
            }
        }
    }
    write.write(buffer, end);
    return nullptr;
}



