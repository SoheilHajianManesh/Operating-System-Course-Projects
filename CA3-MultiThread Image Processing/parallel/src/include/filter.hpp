#ifndef FILTER_HPP
#define FILTER_HPP

#include <vector>

#include "bmp.hpp"

namespace filter{
    void* flipVertical(void* arguments);
    Pixel convolution(Image &sample3x3,std::vector<std::vector<int>> kernel3x3,int row,int col,int rowStart,int rowEnd,int cols);
    void* blur(void* arguments);
    Pixel applyPurpleHaze(const int oldR,const int oldG,const int oldB);
    void* purpleHaze(void* arguments);
    void drawLine(Image& img,Point start,Point end,int rowStart,int rowEnd);
    void* origonalHatch(void* arguments);
}

#endif