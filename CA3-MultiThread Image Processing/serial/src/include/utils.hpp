#ifndef UTILS_HPP
#define UTILS_HPP

#include <algorithm>
#include <vector>


#define R "red"
#define G "green"
#define B "blue"

#define NORMALIZER 16

typedef struct Point {
    int x;
    int y;
}Point;

typedef struct Pixel{
        int red;
        int green;
        int blue;
        Pixel(){
            red=0;
            green=0;
            blue=0;
        }
        Pixel(int red_, int green_, int blue_){
            red=std::max(0,std::min(255,red_));
            green=std::max(0,std::min(255,green_));
            blue=std::max(0,std::min(255,blue_));

        }
}Pixel;


typedef std::vector<std::vector<Pixel>> Image;

#endif