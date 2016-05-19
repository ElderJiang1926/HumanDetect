#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<algorithm>

using namespace std;

#include"cv.h"
#include"highgui.h"
using namespace cv;

struct HOGThreadData
{
			vector<Rect> rectangles;
			vector<Point> locations;
			Mat smallerImgBuf;

			HOGThreadData(){}
};

class  Descriptor
{
public:
     enum { L2Hys=0 };

    Descriptor() : winSize(64,128), blockSize(16,16), blockStride(8,8),
        cellSize(8,8), nbins(9), derivAperture(1), winSigma(-1),
        histogramNormType(L2Hys), L2HysThreshold(0.2), gammaCorrection(true)
    {}

    Descriptor(Size _winSize, Size _blockSize, Size _blockStride,
        Size _cellSize, int _nbins, int _derivAperture=1, double _winSigma=-1,
        int _histogramNormType=L2Hys, double _L2HysThreshold=0.2, bool _gammaCorrection=false)
        : winSize(_winSize), blockSize(_blockSize), blockStride(_blockStride), cellSize(_cellSize),
        nbins(_nbins), derivAperture(_derivAperture), winSigma(_winSigma),
        histogramNormType(_histogramNormType), L2HysThreshold(_L2HysThreshold),
        gammaCorrection(_gammaCorrection)
    {}

    Descriptor(const String& filename)
    {
        load(filename);
    }

    virtual ~Descriptor() {}

    size_t getDescriptorSize() const;
    bool checkDetectorSize() const;
    double getWinSigma() const;

    virtual void setSVMDetector(const vector<float>& _svmdetector);

    virtual bool load(const String& filename, const String& objname=String());
    virtual void save(const String& filename, const String& objname=String()) const;

    virtual void compute(const Mat& img,
                         vector<float>& descriptors,
                         Size winStride=Size(), Size padding=Size(),
                         const vector<Point>& locations=vector<Point>()) const;
    virtual void detect(const Mat& img, vector<Point>& foundLocations,
                        double hitThreshold=0, Size winStride=Size(),
                        Size padding=Size(),
                        const vector<Point>& searchLocations=vector<Point>()) const;
    virtual void detectMultiScale(const Mat& img, vector<Rect>& foundLocations,
                                  double hitThreshold=0, Size winStride=Size(),
                                  Size padding=Size(), double scale=1.05,
                                  int groupThreshold=2) const;
    virtual void computeGradient(const Mat& img, Mat& grad, Mat& angleOfs,
                                 Size paddingTL=Size(), Size paddingBR=Size()) const;

    static vector<float> getDefaultPeopleDetector();

    Size winSize;
    Size blockSize;
    Size blockStride;
    Size cellSize;
    int nbins;
    int derivAperture;
    double winSigma;
    int histogramNormType;
    double L2HysThreshold;
    bool gammaCorrection;
    vector<float> svmDetector;

};

