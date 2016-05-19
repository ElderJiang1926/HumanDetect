#include "stdafx.h"
#include <iostream>   
#include <opencv/cv.h>   
#include <opencv/highgui.h>
#include <opencv/cvaux.h>

//windows的头文件，必须要，不然NuiApi.h用不了  
#include <Windows.h>  
//Kinect for windows 的头文件  
#include "NuiApi.h"  

using namespace std;
using namespace cv;

#include <d3d11.h>  


//最远距离(mm)  
const int MAX_DISTANCE = 3500;
//最近距离(mm)  
const int MIN_DISTANCE = 200;

const LONG m_depthWidth = 640;
const LONG m_depthHeight = 480;
const LONG m_colorWidth = 640;
const LONG m_colorHeight = 480;
const LONG cBytesPerPixel = 4;

BackgroundSubtractorMOG mog(20, 10, 0.5, false);  //高斯背景模型;

int countFlag = 0;//计数标志
Mat frame(m_colorHeight, m_colorWidth, CV_8UC3);
Mat framegray(m_colorHeight, m_colorWidth, CV_8UC1);
Mat foreground(m_colorHeight, m_colorWidth, CV_8UC1);//前景
Mat background(m_colorHeight, m_colorWidth, CV_8UC3);//背景
Mat foreRefer(m_colorHeight, m_colorWidth, CV_8UC1);//前景参考矩阵
Mat backRefer(m_colorHeight, m_colorWidth, CV_8UC3);//背景参考矩阵
Mat depthChange(m_depthHeight, m_depthWidth, CV_8UC1);//标记深度图像变化量
Mat depthRefer(m_depthHeight, m_depthWidth, CV_8UC1);//深度参考矩阵
Mat bodyRefer(m_depthHeight, m_depthWidth, CV_8UC1);//人体参考矩阵
Mat bodyReferDilated(m_depthHeight, m_depthWidth, CV_8UC1);//人体参考矩阵膨胀  
Mat needbgprocess(m_depthHeight, m_depthWidth, CV_8UC1);//彩色空间内需要在提取背景中进行处理的像素点对应的矩阵
Mat needbgprocessDilated(m_depthHeight, m_depthWidth, CV_8UC1);//彩色空间内需要在提取背景中进行处理的像素点对应的矩阵