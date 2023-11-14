// PSNR.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include <math.h>
#include <iostream>
//#include <opencv/cv.h>
//#include </opt/homebrew/Cellar/opencv/4.8.0_6/libopencv2/opencv.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;


int main()
{
    Mat img_in;

    // image 읽고 gray로 바꾸기
    img_in = imread("/Users/songmingyu/Homework_01/Lena_256x256.png");
    cvtColor(img_in, img_in, cv::COLOR_RGB2GRAY);
    
    int newSize_y = 0;
    int newSize_x = 0;


    cout << "크기 설정을 위한 두 정수 입력" << '\n';
    cin >> newSize_x >> newSize_y;
    


    unsigned char *pData;
    pData = (unsigned char *)img_in.data;
    imshow("source img", img_in);
    
    ///////////////////// 처리하기 ///////////////////
    // interPolation 구현
    
    Mat img_new(1024, 1024, CV_8U);
    unsigned char *nData;
    nData = (unsigned char *)img_new.data;

    double rate_y = (double)newSize_y / img_in.cols;
    double rate_x = (double)newSize_x / img_in.rows;


    for(int h=0; h<newSize_y; h++){
        for(int w=0; w<newSize_x; w++){

            int py = h/rate_y;
            int px = w/rate_x;

            double p1 = pData[py * img_in.rows + px];
            double p2 = pData[py * img_in.rows + px+1];
            double p3 = pData[(py+1) * img_in.rows + px];
            double p4 = pData[(py+1) * img_in.rows + px+1];

            double Xrate = w/rate_x - px;
            double Yrate = h/rate_y - py;

            double q1 = (1-Xrate)*p1 + Xrate*p2;
            double q2 = (1-Xrate)*p3 + Xrate*p4;
            double q = (1-Yrate)*q1 + Yrate*q2;

            nData[h*newSize_x + w] = q;
        }
    }

    imshow("output image", img_new);

    /////////////////////////// 처리하기 /////////////////
    /// 회전 구현
    
    double input_angle = 0;

    cout << "회전 각도 입력" << '\n';
    cin >> input_angle;
    
    pData = (unsigned char *)img_in.data;
    
    Mat img_new_rotate(img_in.rows, img_in.cols, CV_8U);
    nData = (unsigned char *)img_new_rotate.data;

    double div_y = img_new_rotate.cols/2;
    double div_x = img_new_rotate.rows/2;

    for(int h=0; h<img_new_rotate.cols; h++)
        for(int w=0; w<img_new_rotate.rows; w++)
            nData[h*img_new_rotate.rows + w] = 255;

    double angle = input_angle * M_PI / 180;

    for(int h=0; h<img_new_rotate.cols; h++){
        for(int w=0; w<img_new_rotate.rows; w++){

            int ty = h - div_y;
            int tx = w - div_x;

            double temp_X = cos(angle)*tx + sin(angle)*ty;
            double temp_Y = -sin(angle)*tx + cos(angle)*ty;

            int new_X = temp_X + div_x;
            int new_Y = temp_Y + div_y;

            if(new_X < 0 || new_X > img_in.rows) continue;
            if(new_Y < 0 || new_Y > img_in.cols) continue;

            int rate = 1;
            int py = new_Y;
            int px = new_X;

            double p1 = pData[py * img_in.rows + px];
            double p2 = pData[py * img_in.rows + px+1];
            double p3 = pData[(py+1) * img_in.rows + px];
            double p4 = pData[(py+1) * img_in.rows + px+1];

            double Xrate = new_X/rate - px;
            double Yrate = new_Y/rate - py;

            double q1 = (1-Xrate)*p1 + Xrate*p2;
            double q2 = (1-Xrate)*p3 + Xrate*p4;
            double q = (1-Yrate)*q1 + Yrate*q2;

            nData[h*img_new_rotate.rows + w] = q;
        }
    }

    imshow("output image_rotate", img_new_rotate);
    
    waitKey(0);
    

    return 0;
}
