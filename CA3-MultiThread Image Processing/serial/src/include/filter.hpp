#ifndef FILTER_HPP
#define FILTER_HPP

#include <vector>

#include "bmp.hpp"

namespace filter{
    void flipVertical(Bmp24 &img);
    Pixel  convolution(Image sample3x3,std::vector<std::vector<int>> kernel3x3);
    void blur(Bmp24 &img);
    Pixel applyPurpleHaze(const int oldR,const int oldG,const int oldB);
    void purpleHaze(Bmp24 &img);
    void drawLine(Image &img,Point start,Point end);
    void origonalHatch(Bmp24 &img);
}

#endif