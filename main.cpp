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
   //double frames = cvGetCaptureProperty(stream, CV_CAP_PROP_FRAME_COUNT);//��ȡ��Ƶ���ж���֡

   double fps = 30.0;//֡�ʿ��Ի�ȡԴ��Ƶ�ļ���
   CvSize size = cvSize(480, 360);
   VideoWriter writer("outDepth1.avi", CV_FOURCC('X', 'V', 'I', 'D'), fps, size, true);
   if (!writer.isOpened())
   {
	   cout << "��ʼ��VideoWriterʧ�ܣ�" << endl;
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
//		input >> imagename;//����ÿһ��ͼƬ·��
//		image = imread(imagename);
//		if (image.empty())break;
//		peopledetect(image, hog, found);
//		imshow("image", image);
//		waitKey(2);
//	}
//	image.release();//��������
//	input.close();//�ر�·���ļ�
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
	//�ҳ����˾���
	for (int i = 0; i < found.size(); i++){
		Rect r = found[i];
		if (r.area()>max_area && ((r.y + r.height / 2)>imgc.rows*0.25)){
			max_area_idx = i;
			max_area = r.area();
		}
	}
	res = max_area_idx;
}
//������
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

	//������;    
	for (i = 0; i<threshold; i++)
	{
		//      if(hist_p[i]==0)    
		//          continue;    
		float percentage = hist_p[i] / total_back;
		entropy_back += -percentage * logf(percentage); // �����Ķ��幫ʽ    
	}
	//ǰ����;    
	for (i = threshold; i<hist.cols; i++)
	{
		//      if(hist_p[i]==0)    
		//      {    
		//          continue;    
		//      }    
		float percentage = hist_p[i] / total_object;
		entropy_object += -percentage * logf(percentage); // �����Ķ��幫ʽ��    
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
//����ֱ��ͼ
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
//����ļ��л���Ƶ
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
//��ⵥ���ļ�
void Detect_Img(cv::vector<cv::Rect> &found,string name){
	Descriptor hog;
	hog.setSVMDetector(Descriptor::getDefaultPeopleDetector());
	//�򿪲�ɫͼ
	Mat imagec = imread(name+"c.jpg");
	//̽������
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
	long sum0 = 0, sum1 = 0; //�洢ǰ���ĻҶ��ܺͺͱ����Ҷ��ܺ�  
	long cnt0 = 0, cnt1 = 0; //ǰ�����ܸ����ͱ������ܸ���  
	double w0 = 0, w1 = 0; //ǰ���ͱ�����ռ����ͼ��ı���  
	double u0 = 0, u1 = 0;  //ǰ���ͱ�����ƽ���Ҷ�  
	double variance = 0; //�����䷽��  
	int i, j;
	double u = 0;
	double maxVariance = 0;
	for (i = 1; i < 256; i++) //һ�α���ÿ������  
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

		u = u0 * w0 + u1 * w1; //ͼ���ƽ���Ҷ�  
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
	//�����ԣ���0��ʼ
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
	/// �趨bin��Ŀ  
	int histSize = 255;

	/// �趨ȡֵ��Χ ( R,G,B) )  
	float range[] = { 0, 255 };
	const float* histRange = { range };

	bool uniform = true; bool accumulate = false;

	Mat hist;

	/// ����ֱ��ͼ:  
	calcHist(&m, 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);

	// ����ֱ��ͼ����  
	int hist_w = 400; int hist_h = 400;
	int bin_w = cvRound((double)hist_w / histSize);

	Mat histImage(hist_w, hist_h, CV_8UC3, Scalar(0, 0, 0));

	/// ��ֱ��ͼ��һ������Χ [ 0, histImage.rows ]  
	normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

	/// ��ֱ��ͼ�����ϻ���ֱ��ͼ  
	for (int i = 1; i < histSize; i++)
	{
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(hist.at<float>(i - 1))),
			Point(bin_w*(i), hist_h - cvRound(hist.at<float>(i))),
			Scalar(0, 0, 255), 2, 8, 0);
	}

	/// ��ʾֱ��ͼ  
	namedWindow("calcHist Demo", CV_WINDOW_AUTOSIZE);
	imshow("calcHist Demo", histImage);
	hist.convertTo(hist, CV_LOAD_IMAGE_GRAYSCALE);
	int j = 0;
	int sum = 0;
	int max = 0;
	int t1 = 0; 
	int t2 = 19;
	//ͳ���ڳ���Ϊ35�������ڵ��������������ֵ��������������
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
//�������
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
	Mat imaged = imread(name+"d.jpg", CV_LOAD_IMAGE_GRAYSCALE);//���ͼ
	//��ʾ���ͼ
	namedWindow("ԭʼ���ͼ", 0);
	imshow("ԭʼ���ͼ", imaged);
	Mat imagec = imread(name+"c.jpg");//��ɫͼ
	//��ʾ��ɫͼ
	namedWindow("ԭʼ��ɫͼ", 0);
	imshow("ԭʼ��ɫͼ", imagec);
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
	//�����ͼ���и���������
	
	Mat cut = imaged(r);
	namedWindow("���ο򸲸ǵ����ͼ", 0);
	imshow("���ο򸲸ǵ����ͼ", cut);

	
	
	//�����������
	cv::rectangle(imaged, r.tl(), r.br(), cv::Scalar(0, 255, 0), 1);
	
	//��ֵ�˲�
	medianBlur(cut, cut, 3);
	int thresh1, thresh2;
	//����õ�������ֵ
	get_thresh(cut, thresh1, thresh2);
	//С�ڵ���ֵ�ĵ����㣻
	threshold(cut, cut, thresh1, 255, CV_THRESH_TOZERO);
	//���ڸ���ֵ�ĵ����㣻
	threshold(cut, cut, thresh2, 255, CV_THRESH_TOZERO_INV);
	//���ڵ���ֵ�ĵ���Ϊ255��
	threshold(cut, cut, thresh1, 255, CV_THRESH_BINARY);
	namedWindow("�и������������", 0);
	imshow("�и������������", cut);
	
	////
	////��ֵ��������otsu�㷨����ȡ����
	////������ֵ
	//MaxEntropy(out, cut);
	////adaptiveThreshold(cut, out, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 25, 5);
	////�ֲ���ֵ��
	//Mat out1;
	//adaptiveThreshold(cut, out1, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 15, 5);

	//��ֵ��,����������ֵȡ�м�Ĳ���

	//���������ǵ���ɫͼ���ӽǾ���
	Merge(cut, r, imagec);
	clearBG(imagec,r);
	namedWindow("���ǵ���ɫͼ��", 0);
	imshow("���ǵ���ɫͼ��", imagec);
    waitKey();
    return 0;

}
