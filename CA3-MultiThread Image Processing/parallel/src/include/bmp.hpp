#ifndef BMP_HPP
#define BMP_HPP

#include <vector>

#include "utils.hpp"

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;
class Bmp24{

#pragma pack(push, 1)
    typedef struct tagBITMAPFILEHEADER {
        WORD bfType;
        DWORD bfSize;
        WORD bfReserved1;
        WORD bfReserved2;
        DWORD bfOffBits;
    } BITMAPFILEHEADER, *PBITMAPFILEHEADER;

    typedef struct tagBITMAPINFOHEADER {
        DWORD biSize;
        LONG biWidth;
        LONG biHeight;
        WORD biPlanes;
        WORD biBitCount;
        DWORD biCompression;
        DWORD biSizeImage;
        LONG biXPelsPerMeter;
        LONG biYPelsPerMeter;
        DWORD biClrUsed;
        DWORD biClrImportant;
    } BITMAPINFOHEADER, *PBITMAPINFOHEADER;
#pragma pack(pop)

public:

    Bmp24(char* fileName);

    bool fillAndAllocate();
    static void* getPixelsFromBMP24(void* arguments);
    static void* writeOutBmp24(void* arguments);

    Pixel& operator()(int row,int col){
        return image[row][col];
    }
    std::vector<Pixel>& operator()(int row){
        return image[row];
    }

    int getWidth(){ return cols;}
    int getHeight(){ return rows;}
    int getEnd(){return  bufferSize;}
    char * getBuffer(){return fileBuffer;}
    Image* getImage(){return &image;}

private:
    PBITMAPFILEHEADER file_header;
    PBITMAPINFOHEADER info_header;
    char* fileBuffer;
    int bufferSize;
    int rows;
    int cols;
    Image image;
    char* fileName;
};

#endif 
