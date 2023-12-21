#include "include/filter.hpp"
#include "include/utils.hpp"
#include <iostream>
using namespace std;

namespace filter{
    void flipVertical(Bmp24 &img){
        int rows = img.getHeight();
        int cols = img.getWidth();
        for(int i = 0; i < rows/2; i++){
            for(int j = 0; j < cols; j++)
                swap(img(i,j),img(rows-i-1,j));
        }
    }
    Pixel convolution(Bmp24 &img,vector<vector<int>> kernel3x3,int row,int col){
        int convR=0;
        int convG=0;
        int convB=0;
        Pixel temp = img(row,col);
        for(int i =0,m=row-1 ; i <3 ; i++,m++){
            for(int j =0,n=col-1 ; j < 3 ; j++,n++){
                if(m < 0 || n < 0 || m >= img.getHeight() || n >= img.getWidth()){
                    convR+=temp.red*kernel3x3[i][j];
                    convG+=temp.green*kernel3x3[i][j];
                    convB+=temp.blue*kernel3x3[i][j];
                }
                else{
                    convR+=img(m,n).red*kernel3x3[i][j];
                    convG+=img(m,n).green*kernel3x3[i][j];
                    convB+=img(m,n).blue*kernel3x3[i][j];
                }
            }
        }  
        Pixel result(convR/NORMALIZER,convG/NORMALIZER,convB/NORMALIZER);
        return result;
    }
    void blur(Bmp24 &img){
        vector<vector<int>> kernel={
            {1,2,1},
            {2,4,2},
            {1,2,1}
        };
        int rows = img.getHeight();
        int cols  = img.getWidth();
        for(int i=0 ; i<rows ; i++){
            for(int j=0 ; j<cols ; j++){
                img(i,j)=convolution(img,kernel,i,j);
            }
        }
    }
    Pixel applyPurpleHaze(const int oldR,const int oldG,const int oldB){
        int red=0.5*oldR+0.3*oldG+0.5*oldB;
        int green=0.16*oldR+0.5*oldG+0.16*oldB;
        int blue=0.6*oldR+0.2*oldG+0.8*oldB;
        Pixel result(red,green,blue);
        return result;
    }
    void purpleHaze(Bmp24 &img){
        int height = img.getHeight();
        int width  = img.getWidth();
        for(int i=0 ; i<height ; i++){
            for(int j=0 ; j<width ; j++){
                img(i,j)=applyPurpleHaze(img(i,j).red,img(i,j).blue,img(i,j).blue);
            }
        }
    }
    void drawLine(Image &img,Point start,Point end){
        int dx=end.x-start.x;
        int dy=end.y-start.y;
        int step = max(abs(dx), abs(dy));

        float incX = static_cast<float>(dx) / step;
        float incY = static_cast<float>(dy) / step;
        
        float x=start.x;
        float y=start.y;
        for(int i=0; i<step ; i++){
            img[x][y]=Pixel(255,255,255);
            x+=incX;
            y+=incY;
        }
    }
    void origonalHatch(Bmp24 &img){
        Point middelLeft={img.getHeight()/2,0};
        Point middleTop={0,img.getWidth()/2};

        Point bottomLeft={img.getHeight()-1,0};
        Point topRight={0,img.getWidth()-1};

        Point middleBottom={img.getHeight()-1,img.getWidth()/2};
        Point middleRight{img.getHeight()/2,img.getWidth()-1};

        drawLine(*img.getImage(),middelLeft,middleTop);
        drawLine(*img.getImage(),bottomLeft,topRight);
        drawLine(*img.getImage(),middleBottom,middleRight);
    }
}