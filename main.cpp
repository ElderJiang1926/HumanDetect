/*Author:tianqi
  Date  :2016.5.16
  ProjectName: HumanDetect
*/
#include"stdafx.h"
#include<iostream>
#include<fstream>

#include<string>

//#include<filesystem>
#include"HogDescriptor.h"


using namespace std;
typedef enum { back, object } entropy_state;
float total;


void peopledetect(Mat& img,Descriptor& hog,cv::vector<cv::Rect >& found );

void videostream_file(string filename,Descriptor& hog,cv::vector<cv::Rect >& found)
{
    int icount=0;
    
	cv::setNumThreads (2);
	VideoCapture stream(filename.c_str());
   //double fps = cvGetCaptureProperty(stream, CV_CAP_PROP_FPS);
   //double frames = cvGetCaptureProperty(stream, CV_CAP_PROP_FRAME_COUNT);//读取视频中有多少帧

   double fps = 30.0;//帧率可以获取源视频文件的
   CvSize size = cvSize(480, 360);
   VideoWriter writer("outDepth1.avi", CV_FOURCC('X', 'V', 'I', 'D'), fps, size, true);
   if (!writer.isOpened())
   {
	   cout << "初始化VideoWriter失败！" << endl;
	   return;
   }
 //  IplImage * frame;
   namedWindow("stream",0);
   Mat image;
   for (;;)
	   {
		//stream.read(image);

	    stream>>image;
		writer.write(image);
		if((++icount)%10!=0) continue;
		if(image.empty ()) break;
		
		peopledetect(image,hog,found);
		imshow("stream",image);
		waitKey(2);
	   }
   writer.release();  
   stream.release();
   

   
}
//void images_go(string filepath, Descriptor& hog, cv::vector<cv::Rect >& found){
//	int icount = 0;
//	cv::setNumThreads(2);
//	ifstream input(filepath,ios::in);
//	if (!input){
//		cout << "open error" << endl;
//		return;
//	}
//	string imagename;
//	namedWindow("image", 0);
//	Mat image;
//	while (!input.eof()){
//		input >> imagename;//输入每一幅图片路径
//		image = imread(imagename);
//		if (image.empty())break;
//		peopledetect(image, hog, found);
//		imshow("image", image);
//		waitKey(2);
//	}
//	image.release();//清理数据
//	input.close();//关闭路径文件
//}


void peopledetect(Mat& img,Descriptor& hog,cv::vector<cv::Rect >& found )
{
	
    double t = (double)cv::getTickCount();
    
    hog.detectMultiScale(img, found, 0, cv::Size(8,8), cv::Size(32,32), 1.02, 2);\
	size_t l=found.size ();
    t = (double)cv::getTickCount() - t;
    printf("Detection time = %gms\n", t*1000./cv::getTickFrequency());
}
void remove_wrong_detect(Mat &imgc,Mat& imgd, cv::vector<cv::Rect >& found,int &res){
	int max_area_idx = 0;
	long max_area = 0;
	//找出带人矩阵
	for (int i = 0; i < found.size(); i++){
		Rect r = found[i];
		if (r.area()>max_area && ((r.y + r.height / 2)>imgc.rows*0.25)){
			max_area_idx = i;
			max_area = r.area();
		}
	}
	res = max_area_idx;
}
//计算熵
float calEntropy(const Mat& hist, int threshold)
{
	float total_back = 0, total_object = 0;
	float entropy_back = 0, entropy_object = 0;
	float entropy = 0;
	int i = 0;

	const float* hist_p = (float*)hist.ptr<float>(0);
	for (i = 0; i<threshold; i++)
	{
		total_back += hist_p[i];
	}
	total_object = total - total_back;

	//背景熵;    
	for (i = 0; i<threshold; i++)
	{
		//      if(hist_p[i]==0)    
		//          continue;    
		float percentage = hist_p[i] / total_back;
		entropy_back += -percentage * logf(percentage); // 能量的定义公式    
	}
	//前景熵;    
	for (i = threshold; i<hist.cols; i++)
	{
		//      if(hist_p[i]==0)    
		//      {    
		//          continue;    
		//      }    
		float percentage = hist_p[i] / total_object;
		entropy_object += -percentage * logf(percentage); // 能量的定义公式；    
	}

	entropy = entropy_object + entropy_back;
	return entropy;
}
void MaxEntropy(Mat &img, Mat hist)
{
	total = sum(hist)[0];
	float MaxEntropyValue = 0.0, MaxEntropyThreshold = 0.0;
	float tmp;
	for (int i = 0; i<hist.cols; i++)
	{
		tmp = calEntropy(hist, i);
		if (tmp>MaxEntropyValue)
		{
			MaxEntropyValue = tmp;
			MaxEntropyThreshold = i;
		}
	}
	threshold(hist, img, MaxEntropyThreshold, 255, CV_THRESH_BINARY);
	namedWindow("thresholdImg", 0);
	imshow("thresholdImg", img);
	//imwrite("D:/thresholdImg.png", img);
	cout << MaxEntropyThreshold << endl;
	cout << MaxEntropyValue << endl;
}
//计算直方图
void Hist(const Mat& image,Mat& d_hist){
	//vector<Mat>d_planes;
	//split(image, d_planes);
	int histsize  = 256;
	//int histHeight = 256;
	float range[] = { 0, 255 };
	const float* histRange[] = { range };
	const int channels = 0;
	calcHist(&image, 1, &channels, Mat(), d_hist, 1, &histsize, &histRange[0], true, false);
}
//检测文件夹或视频
void Detect_Imgs(){
	Descriptor hog;
	hog.setSVMDetector(Descriptor::getDefaultPeopleDetector());
	cv::vector<cv::Rect> found;
	namedWindow("image", 0);
	namedWindow("hist", 0);
	namedWindow("detect", 0);
	Mat imagec, imaged, d_hist;
	ifstream input("LIST.txt", ios::in);
	string filename;
	while (!input.eof()){
		input >> filename;
		imagec = imread("cd\\" + filename);
		input >> filename;
		imaged = imread("cd\\" + filename, CV_LOAD_IMAGE_GRAYSCALE);
		imshow("image", imaged);
		int hist_w = 600; int hist_h = 400;
		int histsize = 256;
		int bin_w = cvRound((double)hist_w / histsize);
		Hist(imaged, d_hist);
		//Mat hist = d_hist.t();
		Mat out;
		adaptiveThreshold(imaged, out, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 3, 5);
		imshow("hist", out);
		peopledetect(imagec, hog, found);
		imshow("detect", imagec);
		cout << filename << endl;
		waitKey(2);
	}
}
//检测单个文件
void Detect_Img(cv::vector<cv::Rect> &found,string name){
	Descriptor hog;
	hog.setSVMDetector(Descriptor::getDefaultPeopleDetector());
	//打开彩色图
	Mat imagec = imread(name+"c.jpg");
	//探测人体
	peopledetect(imagec, hog, found);
	waitKey(2);
	imagec.release();
}
double Otsu(Mat &src)
{
	int height = src.cols;
	int width = src.rows;
	long size = height * width;

	//histogram    
	float histogram[256] = { 0 };
	for (int m = 0; m < height; m++)
	{
		unsigned char* p = (unsigned char*)src.data + src.step * m;
		for (int n = 0; n < width; n++)
		{
			histogram[int(*p++)]++;
		}
	}

	double threshold;
	long sum0 = 0, sum1 = 0; //存储前景的灰度总和和背景灰度总和  
	long cnt0 = 0, cnt1 = 0; //前景的总个数和背景的总个数  
	double w0 = 0, w1 = 0; //前景和背景所占整幅图像的比例  
	double u0 = 0, u1 = 0;  //前景和背景的平均灰度  
	double variance = 0; //最大类间方差  
	int i, j;
	double u = 0;
	double maxVariance = 0;
	for (i = 1; i < 256; i++) //一次遍历每个像素  
	{
		sum0 = 0;
		sum1 = 0;
		cnt0 = 0;
		cnt1 = 0;
		w0 = 0;
		w1 = 0;
		for (j = 0; j < i; j++)
		{
			cnt0 += histogram[j];
			sum0 += j * histogram[j];
		}

		u0 = (double)sum0 / cnt0;
		w0 = (double)cnt0 / size;

		for (j = i; j <= 255; j++)
		{
			cnt1 += histogram[j];
			sum1 += j * histogram[j];
		}

		u1 = (double)sum1 / cnt1;
		w1 = 1 - w0; // (double)cnt1 / size;  

		u = u0 * w0 + u1 * w1; //图像的平均灰度  
		printf("u = %f\n", u);
		//variance =  w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);  
		variance = w0 * w1 *  (u0 - u1) * (u0 - u1);
		if (variance > maxVariance)
		{
			maxVariance = variance;
			threshold = i;
		}
	}

	printf("threshold = %d\n", threshold);
	return threshold;
}
void Merge(Mat cut, Rect r, Mat &src){
	unsigned char element;
	Mat rgb[3];
	Mat m;
	Mat res;
	cut.convertTo(m, CV_LOAD_IMAGE_GRAYSCALE);
	split(src, rgb);
	//经测试，从0开始
	for (int i = 0; i < cut.rows; i++){
		for (int j = 0; j < cut.cols; j++){
			element = cut.at<unsigned char>(i, j);
			if (element){
				cout << element << endl;
			}
			if (element != 255){
				rgb[0].at<unsigned char>(i + r.y, j + r.x) = 0;
				rgb[1].at<unsigned char>(i + r.y, j + r.x) = 0;
				rgb[2].at<unsigned char>(i + r.y, j + r.x) = 0;
			}
		}
	}
	merge(rgb, 3, res);
	src = res;
	rgb[0].release();
	rgb[1].release();
	rgb[2].release();
}
void get_thresh(Mat m, int &thresh1, int &thresh2){
	/// 设定bin数目  
	int histSize = 255;

	/// 设定取值范围 ( R,G,B) )  
	float range[] = { 0, 255 };
	const float* histRange = { range };

	bool uniform = true; bool accumulate = false;

	Mat hist;

	/// 计算直方图:  
	calcHist(&m, 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);

	// 创建直方图画布  
	int hist_w = 400; int hist_h = 400;
	int bin_w = cvRound((double)hist_w / histSize);

	Mat histImage(hist_w, hist_h, CV_8UC3, Scalar(0, 0, 0));

	/// 将直方图归一化到范围 [ 0, histImage.rows ]  
	normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

	/// 在直方图画布上画出直方图  
	for (int i = 1; i < histSize; i++)
	{
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(hist.at<float>(i - 1))),
			Point(bin_w*(i), hist_h - cvRound(hist.at<float>(i))),
			Scalar(0, 0, 255), 2, 8, 0);
	}

	/// 显示直方图  
	namedWindow("calcHist Demo", CV_WINDOW_AUTOSIZE);
	imshow("calcHist Demo", histImage);
	hist.convertTo(hist, CV_LOAD_IMAGE_GRAYSCALE);
	int j = 0;
	int sum = 0;
	int max = 0;
	int t1 = 0; 
	int t2 = 19;
	//统计在长度为35的区间内的像素数量的最大值，求出区间的两端
	for (j = 0; j < 20; ++j){
		sum += (int)hist.at<unsigned char>(j,0);
	}
	max = sum;
	while (j < 255){
		int up = (int)(hist.at<unsigned char>(j, 0));
		int down = ((int)hist.at<unsigned char>(j - 20, 0));
		sum = sum + up-down;
		if (max < sum){
			max = sum;
			t1 = j - 19;
			t2 = j;
		}
		j++;	
	}
	thresh1 = t1;
	thresh2 = t2;
}
//清除背景
void clearBG(Mat &img, Rect r){
	Mat a = Mat(img.cols, img.rows, CV_8UC3, Scalar(0, 0, 0));
	Mat destroi = a(r);
	img(r).convertTo(destroi, destroi.type(), 1, 0);
	img = a;
	a.release();
}
int main(int argc, char** argv)
{
	
	vector<Rect> found;
	/*cd\\12111*/
	string name = "1";
	Detect_Img(found, name);
	Mat imaged = imread(name+"d.jpg", CV_LOAD_IMAGE_GRAYSCALE);//深度图
	//显示深度图
	namedWindow("原始深度图", 0);
	imshow("原始深度图", imaged);
	Mat imagec = imread(name+"c.jpg");//彩色图
	//显示彩色图
	namedWindow("原始彩色图", 0);
	imshow("原始彩色图", imagec);
	int idx;
	remove_wrong_detect(imagec, imaged, found, idx);
	Rect r = found[idx];
	r.x += cvRound(r.width*0.1);
	//r.y -= cvRound(r.height*0.1);
	r.x = r.x > 0 ? r.x : 0;
	r.y = r.y > 0 ? r.y : 0;
	r.width = cvRound(r.width*0.6);
	r.height = cvRound(r.height*0.8);
	if (r.height + r.y > imaged.rows)r.height = imaged.rows - r.y;
	//在深度图中切割有人区域
	
	Mat cut = imaged(r);
	namedWindow("人形框覆盖到深度图", 0);
	imshow("人形框覆盖到深度图", cut);

	
	
	//框出有人区域
	cv::rectangle(imaged, r.tl(), r.br(), cv::Scalar(0, 255, 0), 1);
	
	//中值滤波
	medianBlur(cut, cut, 3);
	int thresh1, thresh2;
	//计算得到两个阈值
	get_thresh(cut, thresh1, thresh2);
	//小于低阈值的点清零；
	threshold(cut, cut, thresh1, 255, CV_THRESH_TOZERO);
	//大于高阈值的点清零；
	threshold(cut, cut, thresh2, 255, CV_THRESH_TOZERO_INV);
	//大于低阈值的点置为255；
	threshold(cut, cut, thresh1, 255, CV_THRESH_BINARY);
	namedWindow("切割出的人形区域", 0);
	imshow("切割出的人形区域", cut);
	
	////
	////二值化，内置otsu算法，提取轮廓
	////计算阈值
	//MaxEntropy(out, cut);
	////adaptiveThreshold(cut, out, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 25, 5);
	////局部二值化
	//Mat out1;
	//adaptiveThreshold(cut, out1, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 15, 5);

	//二值化,根据两个阈值取中间的部分

	//将轮廓覆盖到彩色图像，视角纠正
	Merge(cut, r, imagec);
	clearBG(imagec,r);
	namedWindow("覆盖到彩色图像", 0);
	imshow("覆盖到彩色图像", imagec);
    waitKey();
    return 0;

}
