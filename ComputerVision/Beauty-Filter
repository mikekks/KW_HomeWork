// PSNR.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include <math.h>
#include <iostream>
//#include <opencv/cv.h>
//#include </opt/homebrew/Cellar/opencv/4.8.0_6/libopencv2/opencv.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int N;

unsigned char r = 236;
unsigned char g = 222;
unsigned char b = 214;
unsigned char radius = 90;

int row,col;

unsigned char color_slicing(unsigned char* cur, int y, int x){
    int channel = 3;
    
    int cb = cur[y*col*channel + x*channel + 0];
    int cg = cur[y*col*channel + x*channel + 1];
    int cr = cur[y*col*channel + x*channel + 2];
    
    int tb = (cb - b) * (cb - b);
    int tg = (cg - g) * (cg - g);
    int tr = (cr - r) * (cr - r);
    
    int tmp_radius = radius * radius;
    
    if(tb+tg+tr <= tmp_radius){  // ok 조건
        return 255;
    }
    else{
        return 0;
    }
}

unsigned char cal_matrix_zero(unsigned char* cur, unsigned char* compare, int y, int x, int channel, int type){
    int ret = 0;
    int fy = 1;
    int fx = 1;
    int cnt = (2*N+1) * (2*N+1);
    
    for(int i=y-N; i<=y+N; i++){
        fx = 1;
        for(int j=x-N; j<=x+N; j++){
            if(i<0 || j<0 || i>row || j>col){
                ret += 0;
            }
            else{
                ret += cur[i*col*channel + j*channel + type];
            }
            fx++;
        }
        fy++;
    }
    
    for(int i=y-N; i<=y+N; i++){
        for(int j=x-N; j<=x+N; j++){
            if(i<0 || j<0 || i>row || j>col){
                continue;
            }
            else if(compare[i*col + j] == 0)
                cnt--;
        }
    }
    
    ret /= cnt;
    
    if(ret < 0)
        return 0;
    if(ret > 255)
        return 255;

    return ret;
}

void beauty(){
    Mat img_rgb;
    Mat img_slicing;
    
    img_rgb = imread("/Users/songmingyu/Homework_01/beautyTest.png", cv::IMREAD_COLOR);
    cvtColor(img_rgb, img_slicing, cv::COLOR_RGB2GRAY);


    Mat img_rgb_new(img_rgb.rows, img_rgb.cols, img_rgb.type());
    Mat img_smoothing(img_rgb.rows, img_rgb.cols, img_rgb.type());
    Mat img_final(img_rgb.rows, img_rgb.cols, img_rgb.type());

    row = img_rgb.rows;
    col = img_rgb.cols;
    imshow("source img", img_rgb);

    N = 5;
    
    unsigned char *rgb_Data;
    rgb_Data = (unsigned char *)img_rgb.data;
    
    unsigned char *rgb_slicing;
    rgb_slicing = (unsigned char *)img_slicing.data;
    
    unsigned char *rgb_new;
    rgb_new = (unsigned char *)img_rgb_new.data;
    
    // rgb channel
    for(int h=0; h<row; h++){
        for(int w=0; w<col; w++){
            rgb_slicing[h*col + w] = color_slicing(rgb_Data, h, w);
            if(rgb_slicing[h*col + w] == 255){
                for(int c=0; c<3; c++){
                    rgb_new[h*col*3 + w*3 + c] = rgb_Data[h*col*3 + w*3 + c];
                }
            }
        }
    }
    
    imshow("output slicing", img_slicing);
    imshow("new output", img_rgb_new);


    unsigned char *rgb_smoothing;
    rgb_smoothing = (unsigned char *)img_smoothing.data;
    
    for(int h=0; h<row; h++){
        for(int w=0; w<col; w++){
            for(int c=0; c<3; c++){
                rgb_smoothing[h*col*3 + w*3 + c] = cal_matrix_zero(rgb_new, rgb_slicing, h, w, 3, c);
                
            }
        }
    }
    
    imshow("filtering", img_smoothing);
        
    unsigned char *rgb_final;
    rgb_final = (unsigned char *)img_final.data;
    
    for(int h=0; h<row; h++){
        for(int w=0; w<col; w++){
            if(rgb_slicing[h*col + w] == 0){
                for(int c=0; c<3; c++){
                    rgb_smoothing[h*col*3 + w*3 + c] = rgb_Data[h*col*3 + w*3 + c];
                    
                }
            }
            
        }
    }
    
    imshow("output final", img_smoothing);

}

int main()
{
    
    
    beauty();
    waitKey(0);
    

    return 0;
}
