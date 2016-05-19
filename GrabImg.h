#include "stdafx.h"
#include <iostream>   
#include <opencv/cv.h>   
#include <opencv/highgui.h>
#include <opencv/cvaux.h>

//windows��ͷ�ļ�������Ҫ����ȻNuiApi.h�ò���  
#include <Windows.h>  
//Kinect for windows ��ͷ�ļ�  
#include "NuiApi.h"  

using namespace std;
using namespace cv;

#include <d3d11.h>  


//��Զ����(mm)  
const int MAX_DISTANCE = 3500;
//�������(mm)  
const int MIN_DISTANCE = 200;

const LONG m_depthWidth = 640;
const LONG m_depthHeight = 480;
const LONG m_colorWidth = 640;
const LONG m_colorHeight = 480;
const LONG cBytesPerPixel = 4;

BackgroundSubtractorMOG mog(20, 10, 0.5, false);  //��˹����ģ��;

int countFlag = 0;//������־
Mat frame(m_colorHeight, m_colorWidth, CV_8UC3);
Mat framegray(m_colorHeight, m_colorWidth, CV_8UC1);
Mat foreground(m_colorHeight, m_colorWidth, CV_8UC1);//ǰ��
Mat background(m_colorHeight, m_colorWidth, CV_8UC3);//����
Mat foreRefer(m_colorHeight, m_colorWidth, CV_8UC1);//ǰ���ο�����
Mat backRefer(m_colorHeight, m_colorWidth, CV_8UC3);//�����ο�����
Mat depthChange(m_depthHeight, m_depthWidth, CV_8UC1);//������ͼ��仯��
Mat depthRefer(m_depthHeight, m_depthWidth, CV_8UC1);//��Ȳο�����
Mat bodyRefer(m_depthHeight, m_depthWidth, CV_8UC1);//����ο�����
Mat bodyReferDilated(m_depthHeight, m_depthWidth, CV_8UC1);//����ο���������  
Mat needbgprocess(m_depthHeight, m_depthWidth, CV_8UC1);//��ɫ�ռ�����Ҫ����ȡ�����н��д�������ص��Ӧ�ľ���
Mat needbgprocessDilated(m_depthHeight, m_depthWidth, CV_8UC1);//��ɫ�ռ�����Ҫ����ȡ�����н��д�������ص��Ӧ�ľ���