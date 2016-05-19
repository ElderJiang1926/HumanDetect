#include"GrabImg.h"

int Catch()
{
	//彩色图像  
	Mat image_rgb;
	//深度图像  
	Mat image_depth;

	//创建一个MAT  
	image_rgb.create(480, 640, CV_8UC3);
	image_depth.create(480, 640, CV_8UC1);

	//一个KINECT实例指针  
	INuiSensor*  m_pNuiSensor = NULL;

	if (m_pNuiSensor != NULL)
	{
		return 0;
	}

	//记录当前连接KINECT的数量（为多连接做准备）  
	int iSensorCount;
	//获得当前KINECT的数量  
	HRESULT hr = NuiGetSensorCount(&iSensorCount);


	//按照序列初始化KINETC实例，这里就连接了一个KINECT，所以没有用到循环  
	hr = NuiCreateSensorByIndex(iSensorCount - 1, &m_pNuiSensor);
	//初始化，让其可以接收彩色和深度数据流  
	hr = m_pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX);

	//判断是否出错  
	if (FAILED(hr))
	{
		cout << "NuiInitialize failed" << endl;
		return hr;
	}

	//彩色图像获取下一帧事件  
	HANDLE nextColorFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	//彩色图像事件句柄  
	HANDLE colorStreamHandle = NULL;
	//深度图像获取下一帧事件  
	HANDLE nextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	//深度图像事件句柄  
	HANDLE depthStreamHandle = NULL;

	//实例打开数据流，这里NUI_IMAGE_TYPE_COLOR表示彩色图像  
	hr = m_pNuiSensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, 0, 2, nextColorFrameEvent, &colorStreamHandle);

	if (FAILED(hr))//判断是否提取正确  
	{
		cout << "Could not open color image stream video" << endl;
		m_pNuiSensor->NuiShutdown();
		return hr;
	}

	//实例打开数据流，这里NUI_IMAGE_TYPE_DEPTH表示深度图像  
	hr = m_pNuiSensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, NUI_IMAGE_RESOLUTION_640x480, 0, 2, nextDepthFrameEvent, &depthStreamHandle);

	if (FAILED(hr))//判断是否提取正确  
	{
		cout << "Could not open color image stream video" << endl;
		m_pNuiSensor->NuiShutdown();
		return hr;
	}



	cv::namedWindow("depth", CV_WINDOW_AUTOSIZE);
	moveWindow("depth", 300, 600);
	cv::namedWindow("colorImage", CV_WINDOW_AUTOSIZE);
	moveWindow("colorImage", 0, 200);

	while (1)
	{
		NUI_IMAGE_FRAME pImageFrame_rgb;
		NUI_IMAGE_FRAME pImageFrame_depth;


		if (WaitForSingleObject(nextColorFrameEvent, 0) == 0)
		{
			//从刚才打开数据流的流句柄中得到该帧数据，读取到的数据地址存于pImageFrame  
			hr = m_pNuiSensor->NuiImageStreamGetNextFrame(colorStreamHandle, 0, &pImageFrame_rgb);
			if (FAILED(hr))
			{
				cout << "Could not get color image" << endl;
				m_pNuiSensor->NuiShutdown();
				return -1;
			}

			INuiFrameTexture *pTexture = pImageFrame_rgb.pFrameTexture;
			NUI_LOCKED_RECT lockedRect;

			//提取数据帧到LockedRect，它包括两个数据对象：pitch每行字节数，pBits第一个字节地址  
			//并锁定数据，这样当我们读数据的时候，kinect就不会去修改它  
			pTexture->LockRect(0, &lockedRect, NULL, 0);
			//确认获得的数据是否有效  
			if (lockedRect.Pitch != 0)
			{
				//将数据转换为OpenCV的Mat格式  
				for (int i = 0; i < image_rgb.rows; i++)
				{
					//第i行的指针  
					uchar *prt = image_rgb.ptr(i);

					//每个字节代表一个颜色信息，直接使用uchar  
					uchar *pBuffer = (uchar*)(lockedRect.pBits) + i * lockedRect.Pitch;

					for (int j = 0; j < image_rgb.cols; j++)
					{
						prt[3 * j] = pBuffer[4 * j];//内部数据是4个字节，0-1-2是BGR，第4个现在未使用  
						prt[3 * j + 1] = pBuffer[4 * j + 1];
						prt[3 * j + 2] = pBuffer[4 * j + 2];
					}
				}

				//解除锁定  
				pTexture->UnlockRect(0);
				//释放帧  
				m_pNuiSensor->NuiImageStreamReleaseFrame(colorStreamHandle, &pImageFrame_rgb);
				imshow("colorImage", image_rgb);
			}
		}
		else
		{
			cout << "Buffer length of received texture is bogus\r\n" << endl;
		}



		//深度图像的处理  
		if (WaitForSingleObject(nextDepthFrameEvent, INFINITE) == 0)
		{

			hr = m_pNuiSensor->NuiImageStreamGetNextFrame(depthStreamHandle, 0, &pImageFrame_depth);

			if (FAILED(hr))
			{
				cout << "Could not get color image" << endl;
				NuiShutdown();
				return -1;
			}

			INuiFrameTexture *pTexture = pImageFrame_depth.pFrameTexture;
			NUI_LOCKED_RECT lockedRect;
			pTexture->LockRect(0, &lockedRect, NULL, 0);

			//归一化  
			for (int i = 0; i < image_depth.rows; i++)
			{
				uchar *prt = image_depth.ptr<uchar>(i);

				uchar* pBuffer = (uchar*)(lockedRect.pBits) + i * lockedRect.Pitch;
				//这里需要转换，因为每个深度数据是2个字节，应将BYTE转成USHORT  
				USHORT *pBufferRun = (USHORT*)pBuffer;
				uchar *ptrBody = bodyRefer.ptr<uchar>(i);
				for (int j = 0; j < image_depth.cols; j++)
				{
					prt[j] = (uchar)(255 * (pBufferRun[j] >> 3) / 0x0fff);  //直接将数据归一化处理 

				}
			}
			imshow("depth", image_depth);
			pTexture->UnlockRect(0);

			countFlag++;
			m_pNuiSensor->NuiImageStreamReleaseFrame(depthStreamHandle, &pImageFrame_depth);
		}
		else
		{
			cout << "Buffer length of received texture is bogus\r\n" << endl;
		}

		char colorfilename[12] = "", depthfilename[12] = "";
		sprintf(colorfilename, "%dc.jpg", countFlag);
		sprintf(depthfilename, "%dd.jpg", countFlag);


		if (cvWaitKey(5) == 32)
		{
			imwrite(colorfilename, image_rgb);
			imwrite(depthfilename, image_depth);
		}


		if (cvWaitKey(5) == 27)
			break;

	}


	return 0;
}