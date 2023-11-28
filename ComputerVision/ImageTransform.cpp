#include <math.h>
#include <iostream>
#include <opencv2/opencv.hpp>

#define OUT_SIZE 512

using namespace cv;
using namespace std;

int N;

int row,col;

Mat img_origin;
unsigned char *data_origin;


double m1 = 0.974359;
double m2 = 0.240385;
double m3 = -0.804487;
double m4 = 0.548077;
double t1 = 51.5256;
double t2 = 309.641;


void start(){
    
    img_origin = imread("/Users/songmingyu/Homework_01/lena_t.jpg", cv::IMREAD_GRAYSCALE);
    imshow("origin", img_origin);
    
    
    Mat img_total(OUT_SIZE, OUT_SIZE, img_origin.type());
    
    row = img_origin.rows;
    col = img_origin.cols;

    unsigned char *pData = (unsigned char *)img_origin.data;
    unsigned char *nData = (unsigned char *)img_total.data;
    
    double xRate = (double)OUT_SIZE / col;
    double yRate = (double)OUT_SIZE / row;
    
    
    for(int h=0; h<OUT_SIZE; h++){
        for(int w=0; w<OUT_SIZE; w++){

            double tx = w;
            double ty = h;
            
            double px = (tx*m1 + ty*m2 + t1);
            double py = (tx*m3 + ty*m4 + t2);
            
            int nx = px;
            int ny = py;
            
            
            if(nx < 0 || ny < 0 || nx > col || ny > row) continue;
            
            double p1 = pData[ny * col + nx];
            double p2 = pData[ny * col + nx+1];
            double p3 = pData[(ny+1) * col + nx];
            double p4 = pData[(ny+1) * col + nx+1];

            double Xrate = px - nx;
            double Yrate = py - ny;

            double q1 = (1-Xrate)*p1 + Xrate*p2;
            double q2 = (1-Xrate)*p3 + Xrate*p4;
            double q = (1-Yrate)*q1 + Yrate*q2;

            nData[h*OUT_SIZE + w] = q;
            
        }
    }
    
    imshow("result", img_total);
    //imwrite("/Users/songmingyu/Desktop/컴퓨터비전/hw6_2018321006/result.jpg", img_total);

}

int main()
{
    
    start();
    waitKey(0);
    

    return 0;
}
