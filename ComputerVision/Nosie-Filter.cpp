#include <math.h>
#include <iostream>
//#include <opencv/cv.h>
//#include </opt/homebrew/Cellar/opencv/4.8.0_6/libopencv2/opencv.hpp>
#include <opencv2/opencv.hpp>

#define FILTER_SIZE 1

using namespace cv;
using namespace std;

int N;
int divider;

int row,col;

Mat img_origin;
unsigned char *data_origin;


Mat applySaltPepperNoise(Mat& src, double ratio){
    unsigned char *result_SaltPepper = (unsigned char *)src.data;
    int noise_pixel_count = (int)((double)(row*col)*ratio);
    
    for(int h=0; h<row; h++){
        for(int w=0; w<col; w++){
            result_SaltPepper[h*col + w] = data_origin[h*col + w];
        }
    }
    
    for(int i=0; i<noise_pixel_count; i++){
        int r = rand() % row;
        int c = rand() % col;
        
        int value = rand() % 2 == 1 ? 255 : 0;
        result_SaltPepper[r*col + c] = value;
    }
    
    return src;
}

Mat applyGaussianNoise(Mat& src, double mean, double variance){
    unsigned char *result_gaussian = (unsigned char *)src.data;
    
    Mat filter_gaussian(src.size(), CV_16S);
    randn(filter_gaussian, Scalar::all(mean), Scalar::all(variance));
    char *data_gaussian = (char *)filter_gaussian.data;

    
    for(int h=0; h<row; h++){
        for(int w=0; w<col; w++){
            int tmp = data_origin[h*col + w];
            int tmp2 = data_gaussian[h*col + w];
            
            if(tmp + tmp2 > 255){
                result_gaussian[h*col + w] = 255;
            }
            else if(tmp + tmp2 < 0){
                result_gaussian[h*col + w] = 0;
            }
            else{
                result_gaussian[h*col + w] = data_origin[h*col + w] + data_gaussian[h*col + w];
            }
            
        }
    }
    
    return src;
}



int calculateMean(unsigned char* cur, int y, int x, int channel, int type){
    int ret = 0;
    int divider = (2*FILTER_SIZE+1) * (2*FILTER_SIZE+1);
    
    for(int h=y-FILTER_SIZE; h<=y+FILTER_SIZE; h++){
        for(int w=x-FILTER_SIZE; w<=x+FILTER_SIZE; w++){
            if(h<0 || w<0 || h>row || w>col){
                continue;
            }
            ret += cur[h*col*channel + w*channel + type];
        }
    }
    
    ret /= divider;
    
    if(ret > 255)
        return 255;
    
    return ret;
}

Mat& meanFilter(Mat& src, Mat& dst){
    unsigned char *sData = (unsigned char *)src.data;
    unsigned char *result_mean = (unsigned char *)dst.data;
    
    for(int h=0; h<row; h++){
        for(int w=0; w<col; w++){
            result_mean[h*col + w] = calculateMean(sData, h, w, 1, 0);
        }
    }
    
    return dst;
}


unsigned char calculateMedian(unsigned char* cur, int y, int x, int channel, int type){
    vector<int> arr;
    
    for(int h=y-FILTER_SIZE; h<=y+FILTER_SIZE; h++){
        for(int w=x-FILTER_SIZE; w<=x+FILTER_SIZE; w++){
            if(h<0 || w<0 || h>row || w>col){
                arr.push_back(0);
            }
            else{
                arr.push_back(cur[h*col+w]);
            }
        }
    }
    
    sort(arr.begin(), arr.end());

    if(cur[y*col+x] > arr[0] && cur[y*col+x] < arr[8]){
        return cur[y*col+x];
    }
    
    return arr[4];
}

Mat& medianFilter(Mat& src, Mat& dst){
    
    unsigned char *sData = (unsigned char *)src.data;
    unsigned char *result_median = (unsigned char *)dst.data;
    
    for(int h=0; h<row; h++){
        for(int w=0; w<col; w++){
            result_median[h*col + w] = calculateMedian(sData, h, w, 1, 0);
        }
    }
    
    return dst;
}

void start(){
    
    img_origin = imread("/Users/songmingyu/Homework_01/Lena512.jpg", cv::IMREAD_COLOR);
    cvtColor(img_origin, img_origin, cv::COLOR_RGB2GRAY);
    imshow("output : origin", img_origin);
    
    
    // 가우시안 노이즈 영상 담을 Mat
    Mat img_gaussian_low(img_origin.rows, img_origin.cols, img_origin.type());
    Mat img_gaussian_high(img_origin.rows, img_origin.cols, img_origin.type());
    
    // Salt & Pepper 노이즈 영상 담을 Mat
    Mat img_saltPepper_low(img_origin.rows, img_origin.cols, img_origin.type());
    Mat img_saltPepper_high(img_origin.rows, img_origin.cols, img_origin.type());
    
    // mean filter 결과 영상 Mat
    Mat img_gaussian_low_mean(img_origin.rows, img_origin.cols, img_origin.type());
    Mat img_gaussian_high_mean(img_origin.rows, img_origin.cols, img_origin.type());
    Mat img_saltPepper_low_mean(img_origin.rows, img_origin.cols, img_origin.type());
    Mat img_saltPepper_high_mean(img_origin.rows, img_origin.cols, img_origin.type());
    
    // median filter 결과 영상 Mat
    Mat img_gaussian_low_median(img_origin.rows, img_origin.cols, img_origin.type());
    Mat img_gaussian_high_median(img_origin.rows, img_origin.cols, img_origin.type());
    Mat img_saltPepper_low_median(img_origin.rows, img_origin.cols, img_origin.type());
    Mat img_saltPepper_high_median(img_origin.rows, img_origin.cols, img_origin.type());
    
    row = img_origin.rows;
    col = img_origin.cols;
    data_origin = (unsigned char *)img_origin.data;

    
    // 가우시안 노이즈 적용
    img_gaussian_low = applyGaussianNoise(img_gaussian_low, 0, 10);
    img_gaussian_high = applyGaussianNoise(img_gaussian_high, 0, 30);
    imshow("noise-output : gausssian_noise_low", img_gaussian_low);
    imshow("noise-output : gausssian_noise_high", img_gaussian_high);
    
    // Salt & Pepper 노이즈 적용
    img_saltPepper_low = applySaltPepperNoise(img_saltPepper_low, 0.05);
    img_saltPepper_high = applySaltPepperNoise(img_saltPepper_high, 0.1);
    imshow("noise-output : SaltPepper_noise_low", img_saltPepper_low);
    imshow("noise-output : SaltPepper_noise_high", img_saltPepper_high);

    
    
//    Mat img_result(img_origin.rows, img_origin.cols, img_origin.type());
    
    // mean Filter 적용
    img_gaussian_low_mean = meanFilter(img_gaussian_low, img_gaussian_low_mean);
    imshow("mean-output : gausssian_noise_low", img_gaussian_low_mean);
    
    img_gaussian_high_mean = meanFilter(img_gaussian_high, img_gaussian_high_mean);
    imshow("mean-output : gausssian_noise_high", img_gaussian_high_mean);

    img_saltPepper_low_mean = meanFilter(img_saltPepper_low, img_saltPepper_low_mean);
    imshow("mean-output : SaltPepper_noise_low", img_saltPepper_low_mean);

    img_saltPepper_high_mean = meanFilter(img_saltPepper_high, img_saltPepper_high_mean);
    imshow("mean-output : SaltPepper_noise_high", img_saltPepper_high_mean);

    
    // median Filter 적용
    img_gaussian_low_median = medianFilter(img_gaussian_low, img_gaussian_low_median);
    imshow("median-output : gausssian_noise_low", img_gaussian_low_median);
    
    img_gaussian_high_median = medianFilter(img_gaussian_high, img_gaussian_high_median);
    imshow("median-output : gausssian_noise_high", img_gaussian_high_median);

    img_saltPepper_low_median = medianFilter(img_saltPepper_low, img_saltPepper_low_median);
    imshow("median-output : SaltPepper_noise_low", img_saltPepper_low_median);

    img_saltPepper_high_median = medianFilter(img_saltPepper_high, img_saltPepper_high_median);
    imshow("median-output : SaltPepper_noise_high", img_saltPepper_high_median);
    
}

int main()
{
    
    start();
    waitKey(0);
    

    return 0;
}



