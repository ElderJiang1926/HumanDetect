#include"GrabImg.h"

int Catch()
{
	//��ɫͼ��  
	Mat image_rgb;
	//���ͼ��  
	Mat image_depth;

	//����һ��MAT  
	image_rgb.create(480, 640, CV_8UC3);
	image_depth.create(480, 640, CV_8UC1);

	//һ��KINECTʵ��ָ��  
	INuiSensor*  m_pNuiSensor = NULL;

	if (m_pNuiSensor != NULL)
	{
		return 0;
	}

	//��¼��ǰ����KINECT��������Ϊ��������׼����  
	int iSensorCount;
	//��õ�ǰKINECT������  
	HRESULT hr = NuiGetSensorCount(&iSensorCount);


	//�������г�ʼ��KINETCʵ���������������һ��KINECT������û���õ�ѭ��  
	hr = NuiCreateSensorByIndex(iSensorCount - 1, &m_pNuiSensor);
	//��ʼ����������Խ��ղ�ɫ�����������  
	hr = m_pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX);

	//�ж��Ƿ����  
	if (FAILED(hr))
	{
		cout << "NuiInitialize failed" << endl;
		return hr;
	}

	//��ɫͼ���ȡ��һ֡�¼�  
	HANDLE nextColorFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	//��ɫͼ���¼����  
	HANDLE colorStreamHandle = NULL;
	//���ͼ���ȡ��һ֡�¼�  
	HANDLE nextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	//���ͼ���¼����  
	HANDLE depthStreamHandle = NULL;

	//ʵ����������������NUI_IMAGE_TYPE_COLOR��ʾ��ɫͼ��  
	hr = m_pNuiSensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, 0, 2, nextColorFrameEvent, &colorStreamHandle);

	if (FAILED(hr))//�ж��Ƿ���ȡ��ȷ  
	{
		cout << "Could not open color image stream video" << endl;
		m_pNuiSensor->NuiShutdown();
		return hr;
	}

	//ʵ����������������NUI_IMAGE_TYPE_DEPTH��ʾ���ͼ��  
	hr = m_pNuiSensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, NUI_IMAGE_RESOLUTION_640x480, 0, 2, nextDepthFrameEvent, &depthStreamHandle);

	if (FAILED(hr))//�ж��Ƿ���ȡ��ȷ  
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
			//�ӸղŴ���������������еõ���֡���ݣ���ȡ�������ݵ�ַ����pImageFrame  
			hr = m_pNuiSensor->NuiImageStreamGetNextFrame(colorStreamHandle, 0, &pImageFrame_rgb);
			if (FAILED(hr))
			{
				cout << "Could not get color image" << endl;
				m_pNuiSensor->NuiShutdown();
				return -1;
			}

			INuiFrameTexture *pTexture = pImageFrame_rgb.pFrameTexture;
			NUI_LOCKED_RECT lockedRect;

			//��ȡ����֡��LockedRect���������������ݶ���pitchÿ���ֽ�����pBits��һ���ֽڵ�ַ  
			//���������ݣ����������Ƕ����ݵ�ʱ��kinect�Ͳ���ȥ�޸���  
			pTexture->LockRect(0, &lockedRect, NULL, 0);
			//ȷ�ϻ�õ������Ƿ���Ч  
			if (lockedRect.Pitch != 0)
			{
				//������ת��ΪOpenCV��Mat��ʽ  
				for (int i = 0; i < image_rgb.rows; i++)
				{
					//��i�е�ָ��  
					uchar *prt = image_rgb.ptr(i);

					//ÿ���ֽڴ���һ����ɫ��Ϣ��ֱ��ʹ��uchar  
					uchar *pBuffer = (uchar*)(lockedRect.pBits) + i * lockedRect.Pitch;

					for (int j = 0; j < image_rgb.cols; j++)
					{
						prt[3 * j] = pBuffer[4 * j];//�ڲ�������4���ֽڣ�0-1-2��BGR����4������δʹ��  
						prt[3 * j + 1] = pBuffer[4 * j + 1];
						prt[3 * j + 2] = pBuffer[4 * j + 2];
					}
				}

				//�������  
				pTexture->UnlockRect(0);
				//�ͷ�֡  
				m_pNuiSensor->NuiImageStreamReleaseFrame(colorStreamHandle, &pImageFrame_rgb);
				imshow("colorImage", image_rgb);
			}
		}
		else
		{
			cout << "Buffer length of received texture is bogus\r\n" << endl;
		}



		//���ͼ��Ĵ���  
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

			//��һ��  
			for (int i = 0; i < image_depth.rows; i++)
			{
				uchar *prt = image_depth.ptr<uchar>(i);

				uchar* pBuffer = (uchar*)(lockedRect.pBits) + i * lockedRect.Pitch;
				//������Ҫת������Ϊÿ�����������2���ֽڣ�Ӧ��BYTEת��USHORT  
				USHORT *pBufferRun = (USHORT*)pBuffer;
				uchar *ptrBody = bodyRefer.ptr<uchar>(i);
				for (int j = 0; j < image_depth.cols; j++)
				{
					prt[j] = (uchar)(255 * (pBufferRun[j] >> 3) / 0x0fff);  //ֱ�ӽ����ݹ�һ������ 

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