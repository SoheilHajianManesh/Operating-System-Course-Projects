#include "include/filter.hpp"
#include "include/utils.hpp"

using namespace std;

namespace filter{
    void* flipVertical(void* arguments){
        ThreadData* args=(ThreadData*) arguments;
        int colStart=args->colStart;
        int colEnd=args->colEnd;
        int rows = args->rows;
        Image* img=args->image;
        for(int i = 0; i < rows/2; i++){
            for(int j=colStart; j <colEnd;j++){
                swap((*img)[i][j],(*img)[rows-i-1][j]);}
        }
        return nullptr;
    }
    Pixel convolution(Image &img,vector<vector<int>> kernel3x3,int row,int col,int rowStart,int rowEnd,int cols){
        int convR=0;
        int convG=0;
        int convB=0;
        Pixel temp = img[row][col];
        for(int i =0,m=row-1 ; i <3 ; i++,m++){
            for(int j =0,n=col-1 ; j < 3 ; j++,n++){
                if(m < rowStart || n < 0 || m >= rowEnd || n >= cols){
                    convR+=temp.red*kernel3x3[i][j];
                    convG+=temp.green*kernel3x3[i][j];
                    convB+=temp.blue*kernel3x3[i][j];
                }
                else{
                    convR+=img[m][n].red*kernel3x3[i][j];
                    convG+=img[m][n].green*kernel3x3[i][j];
                    convB+=img[m][n].blue*kernel3x3[i][j];
                }
            }
        }  
        Pixel result(convR/NORMALIZER,convG/NORMALIZER,convB/NORMALIZER);
        return result;
    }
    void* blur(void* arguments){
        vector<vector<int>> kernel={
            {1,2,1},
            {2,4,2},
            {1,2,1}
        };
        ThreadData* args=(ThreadData*) arguments;
        int rowStart=args->rowStart;
        int rowEnd=args->rowEnd;
        Image* img=args->image;
        int cols=args->cols;
        for(int i=rowStart ; i<rowEnd ; i++){
            for(int j=0 ; j<cols ; j++){
                (*img)[i][j]=convolution(*img,kernel,i,j,rowStart,rowEnd,cols);
            }
        }
        return nullptr;
    }
    Pixel applyPurpleHaze(const int oldR,const int oldG,const int oldB){
        int red=0.5*oldR+0.3*oldG+0.5*oldB;
        int green=0.16*oldR+0.5*oldG+0.16*oldB;
        int blue=0.6*oldR+0.2*oldG+0.8*oldB;
        Pixel result(red,green,blue);
        return result;
    }
    void* purpleHaze(void* arguments){
        ThreadData* args=(ThreadData*) arguments;
        int colStart=args->colStart;
        int colEnd=args->colEnd;
        int rows = args->rows;
        Image* img=args->image;
        for(int i=0 ; i<rows ; i++){
            for(int j=colStart ; j<colEnd ; j++){
                (*img)[i][j]=applyPurpleHaze((*img)[i][j].red,(*img)[i][j].blue,(*img)[i][j].blue);
            }
        }
        return nullptr;
    }
    void drawLine(Image &img,Point start,Point end,int rowStart,int rowEnd){
        int dx=end.x-start.x;
        int dy=end.y-start.y;
        int step = max(abs(dx), abs(dy));

        float incX = static_cast<float>(dx) / step;
        float incY = static_cast<float>(dy) / step;
        
        float x=start.x;
        float y=start.y;
        for(int i=0; i<step ; i++){
            if(x<rowStart||x>=rowEnd){
                x+=incX;
                y+=incY;
                continue;
            }
            img[x][y]=Pixel(255,255,255);
            x+=incX;
            y+=incY;
        }
    }
    void* origonalHatch(void* arguments){
        ThreadData* args=(ThreadData*) arguments;
        int rowStart=args->rowStart;
        int rowEnd=args->rowEnd;
        int rows=args->rows;
        int cols=args->cols;
        Image* img=args->image;
        Point middelLeft={rows/2,0};
        Point middleTop={0,cols/2};

        Point bottomLeft={rows-1,0};
        Point topRight={0,cols-1};

        Point middleBottom={rows-1,cols/2};
        Point middleRight{rows/2,cols-1};

        drawLine(*img,middelLeft,middleTop,rowStart,rowEnd);
        drawLine(*img,bottomLeft,topRight,rowStart,rowEnd);
        drawLine(*img,middleBottom,middleRight,rowStart,rowEnd);
        return nullptr;
    }
}