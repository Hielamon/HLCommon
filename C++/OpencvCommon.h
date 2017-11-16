#pragma once

#include "TraverFolder.h"
#include "Monitor.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <memory>


#define H_FLOAT_MAX std::numeric_limits<float>::max()
#define H_FLOAT_MIN std::numeric_limits<float>::min()
#define H_DOUBLE_MAX std::numeric_limits<double>::max()
#define H_DOUBLE_MIN std::numeric_limits<double>::min()

////////////////////////////////////////////////////////////////////
//The functions Related to the Size

template <class T>
inline bool CheckInSize(const cv::Point_<T> &pt, const cv::Size &size)
{
	return pt.x >= 0 && pt.y >= 0 && pt.x <= (size.width - 1) && pt.y <= (size.height - 1);
}

inline void resizeShow(const std::string &name, const cv::Mat &img, bool sizeToScreen = true)
{
	if (sizeToScreen)
	{
		int w = img.cols;
		int h = img.rows;
		FitSizeToScreen(w, h);

		cv::Mat resizedImg;
		cv::resize(img, resizedImg, cv::Size(w, h));
		cv::imshow(name, resizedImg);
	}
	else
	{
		cv::imshow(name, img);
	}
}


////////////////////////////////////////////////////////////////////
//The functions Related to Random variable

inline cv::Scalar RandomColor()
{
	uchar b = rand() % 256;
	uchar g = rand() % 256;
	uchar r = rand() % 256;
	return cv::Scalar(b, g, r);
}

inline cv::Vec3d RandomAxis()
{
	double theta = CV_2PI * (rand() / double(RAND_MAX));
	double phi = CV_PI * (rand() / double(RAND_MAX));
	/*double theta = CV_PI * 0.5;
	double phi = CV_PI * 0.5;*/
	return cv::Vec3d(sin(phi)*cos(theta), sin(phi)*sin(theta), cos(phi));
}

template <class T>
inline T RandomInRange(T minValue, T maxValue)
{
	assert(minValue <= maxValue);
	double ratio = rand() / (double(RAND_MAX) + 1);
	return minValue + ratio * (maxValue - minValue);
}


////////////////////////////////////////////////////////////////////
//The functions Related to Images Loading

inline bool LoadSameSizeImages(std::vector<cv::Mat> &images, const std::string &dir, const std::string &image_suffix = "jpg")
{
	std::vector<std::string> filelist, templist;
	{
		TraverFolder traver;
		traver.setFolderPath(dir);
		if (!traver.IsFind())
		{
			std::cout << "cannot open the folder " << dir << std::endl;
			return false;
		}
		traver.getFileFullPath(templist, image_suffix);
		for (size_t i = 0; i < templist.size(); i++)
		{
			if (templist[i].find(image_suffix) != std::string::npos)
				filelist.push_back(templist[i]);
		}
		if (filelist.size() == 0)
		{
			std::cerr << "cannot find any images with image_suffix" << std::endl;
			return false;
		}
	}
	cv::Size image_size;


	//¼ÓÔØÍ¼Æ¬
	int num_images = 0;
	std::cout << "\n.............Start to load images............." << std::endl;
	for (size_t i = 0; i < filelist.size(); i++, num_images++)
	{
		cv::Mat temp_image = cv::imread(filelist[i]);
		if (i != 0 && temp_image.size() != image_size)
			break;
		if (i == 0)image_size = temp_image.size();
		images.push_back(temp_image);

	}
	return true;
}

inline bool LoadDiffSizeImages(std::vector<cv::Mat> &images, const std::string &dir, const std::string &image_suffix = "jpg")
{
	std::vector<std::string> vFileName;
	TraverFolder traver(dir);
	if (!traver.IsFind())
	{
		std::cout << "cannot open the folder " << dir << std::endl;
		return false;
	}
	traver.getFileFullPath(vFileName, image_suffix);

	std::for_each(vFileName.begin(), vFileName.end(),
				  [&images](std::string &s) {
		cv::Mat img = cv::imread(s);
		if (img.empty())
		{
			std::cerr << "Failed to load the image with name " << s << std::endl;
			return false;
		}
		images.push_back(img);
	});
	return true;
}


////////////////////////////////////////////////////////////////////
//The functions Related to ROI

template <class T>
bool GetOverlapRoi(const cv::Rect_<T> &roi1, const cv::Rect_<T> &roi2, cv::Rect_<T> &resultRoi)
{
	cv::Point_<T> tl, br;
	cv::Point_<T> br1 = roi1.br(), br2 = roi2.br();
	tl.x = roi1.x > roi2.x ? roi1.x : roi2.x;
	tl.y = roi1.y > roi2.y ? roi1.y : roi2.y;
	br.x = br1.x < br2.x ? br1.x : br2.x;
	br.y = br1.y < br2.y ? br1.y : br2.y;
	if (tl.x > br.x || tl.y > br.y) { return false; }
	else
	{
		resultRoi = cv::Rect_<T>(tl, br);
		return true;
	}
}

template <class T>
cv::Rect_<T> GetUnionRoi(const cv::Rect_<T> &roi1, const cv::Rect_<T> &roi2)
{
	cv::Point_<T> tl, br;
	cv::Point_<T> br1 = roi1.br(), br2 = roi2.br();
	tl.x = roi1.x < roi2.x ? roi1.x : roi2.x;
	tl.y = roi1.y < roi2.y ? roi1.y : roi2.y;
	br.x = br1.x > br2.x ? br1.x : br2.x;
	br.y = br1.y > br2.y ? br1.y : br2.y;
	return cv::Rect_<T>(tl, br);
}

////////////////////////////////////////////////////////////////////
//The functions Related to Drawing

//Draw the Grid on the Img, if the total size of grid is lager than the input image, we resize the origin image
//lineW defines the line width
//drawGridPoint defines whether to draw the gridPoint and pRadius control the radius of grid point
//Return Result ROI
inline cv::Rect DrawGrid(cv::Mat &img, const cv::Point& gridDim, const cv::Size &gridSize, int lineW = 1, int pRadius = 1, bool drawGridPoint = true)
{
	assert(img.type() == CV_8UC3);
	int padding = lineW;
	cv::Size resultSize(gridDim.x * gridSize.width, gridDim.y * gridSize.height);
	resultSize.width = std::max(img.cols, resultSize.width) + padding;
	resultSize.height = std::max(img.rows, resultSize.height) + padding;

	cv::Mat result(resultSize, CV_8UC3, cv::Scalar(0));
	cv::Rect originROI(0, 0, img.cols, img.rows);
	img.copyTo(result(originROI));

	cv::Scalar rColor(255, 0, 255), cColor(0, 255, 255), pColor(255, 0, 0);
	for (int i = 0, x = 0; i <= gridDim.x; i++, x += gridSize.width)
	{
		cv::Point start(x, 0), end(x, resultSize.height - 1);
		cv::line(result, start, end, cColor, lineW, cv::LINE_AA);
	}

	for (int i = 0, y = 0; i <= gridDim.y; i++, y += gridSize.height)
	{
		cv::Point start(0, y), end(resultSize.width - 1, y);
		cv::line(result, start, end, rColor, lineW, cv::LINE_AA);
	}

	if (drawGridPoint)
	{
		for (int i = 0, x = 0; i < gridDim.x + 1; i++, x += gridSize.width)
		{
			for (int j = 0, y = 0; j < gridDim.y + 1; j++, y += gridSize.height)
			{
				cv::circle(result, cv::Point(x, y), pRadius, pColor, -1, cv::LINE_AA);
			}
		}
	}

	img = result;

	return cv::Rect(0, 0, resultSize.width, resultSize.height);
}

//Draw the Grid on the Img, which vertices maybe twisty
//Return Result ROI
inline cv::Rect DrawGridVertices(cv::Mat &img, cv::Rect imgROI, const std::vector<cv::Point2d> &vVertices, const cv::Point& gridDim, int lineW = 1, int pRadius = 1, bool drawGridPoint = true)
{
	assert(img.type() == CV_8UC3);
	assert(vVertices.size() == (gridDim.x + 1) * (gridDim.y + 1));
	assert(img.cols == imgROI.width && img.rows == imgROI.height);
	cv::Point gridTl(vVertices[0]), gridBr = gridTl;
	for (int i = 0; i < vVertices.size(); i++)
	{
		const cv::Point2d &pt = vVertices[i];
		if (gridTl.x > pt.x)gridTl.x = floor(pt.x);
		if (gridTl.y > pt.y)gridTl.y = floor(pt.y);
		if (gridBr.x < pt.x)gridBr.x = ceil(pt.x);
		if (gridBr.y < pt.y)gridBr.y = ceil(pt.y);
	}

	cv::Rect gridROI(gridTl, gridBr);
	cv::Rect originROI(imgROI);
	cv::Rect resultROI = GetUnionRoi(gridROI, originROI);
	cv::Point shiftPt = -resultROI.tl();
	originROI.x += shiftPt.x; originROI.y += shiftPt.y;
	gridROI.x += shiftPt.x; gridROI.y += shiftPt.y;
	int padding = lineW;
	cv::Size resultSize(resultROI.width + padding, resultROI.height + padding);
	cv::Mat result(resultSize, CV_8UC3, cv::Scalar(0));
	
	img.copyTo(result(originROI));
	resultROI.width += padding;
	resultROI.height += padding;

	cv::Scalar rColor(255, 0, 255), cColor(0, 255, 255), pColor(255, 0, 0);
	for (int i = 0, vIdx = 0; i <= gridDim.y; i++)
	{
		for (int j = 0; j <= gridDim.x; j++, vIdx++)
		{
			cv::Point start = vVertices[vIdx];
			if (j < gridDim.x)
			{
				cv::Point end = vVertices[vIdx + 1];
				cv::line(result, start + shiftPt, end + shiftPt, rColor, lineW, cv::LINE_AA);
			}

			if (i < gridDim.y)
			{
				cv::Point end = vVertices[vIdx + gridDim.x + 1];
				cv::line(result, start + shiftPt, end + shiftPt, cColor, lineW, cv::LINE_AA);
			}
		}
	}


	if (drawGridPoint)
	{
		for (size_t i = 0; i < vVertices.size(); i++)
		{
			cv::circle(result, cv::Point(vVertices[i]) + shiftPt, pRadius, pColor, -1, cv::LINE_AA);
		}
	}

	img = result;
	return resultROI;
}

template <class T>
bool PointHTransform(const cv::Point_<T> &srcpt, const cv::Mat &H, cv::Point_<T> &dstpt)
{
	const double *Hptr = reinterpret_cast<const double *>(H.data);
	T z = srcpt.x * Hptr[6] + srcpt.y * Hptr[7] + Hptr[8];
	if (abs(z) <= std::numeric_limits<T>::min())
	{
		return false;
	}
	else
	{
		T zinv = 1.0 / z;
		dstpt.x = zinv*(srcpt.x * Hptr[0] + srcpt.y * Hptr[1] + Hptr[2]);
		dstpt.y = zinv*(srcpt.x * Hptr[3] + srcpt.y * Hptr[4] + Hptr[5]);
		return true;
	}
}

//centering the points and scaling to sqr(2), return regularized points and inverse transformation TInv
template <class T>
void GetRegularizedPoints(std::vector<cv::Point_<T>> &srcPts, std::vector<cv::Point_<T>> &dstPts,
						  cv::Mat &TInv)
{
	int pointsNum = srcPts.size();
	assert(pointsNum > 0);

	dstPts.resize(pointsNum);
	cv::Point_<T> central(0, 0);

	for (size_t i = 0, j = 1; i < pointsNum; i++)
	{
		cv::Point_<T> &pt = srcPts[i];

		T a1 = 1.0 / j;
		T a2 = (j - 1) * a1;

		central = pt*a1 + central*a2;
		j++;
	}

	cv::Point_<T> avgRadius(0, 0);
	for (size_t i = 0, j = 1; i < pointsNum; i++, j++)
	{
		dstPts[i] = srcPts[i] - central;
		cv::Point_<T> dist(std::abs(dstPts[i].x), std::abs(dstPts[i].y));

		T a1 = 1.0 / j;
		T a2 = (j - 1) * a1;

		avgRadius = dist*a1 + avgRadius*a2;
	}

	T scaleX = (T)1 / avgRadius.x;
	T scaleY = (T)1 / avgRadius.y;

	for (size_t i = 0; i < pointsNum; i++)
	{
		dstPts[i].x *= scaleX;
		dstPts[i].y *= scaleY;
	}

	T scaleinvX = (T)1.0 / scaleX;
	T scaleinvY = (T)1.0 / scaleY;

	TInv = cv::Mat::eye(3, 3, CV_64F);
	TInv.at<double>(0, 0) = scaleinvX;
	TInv.at<double>(1, 1) = scaleinvY;
	TInv.at<double>(0, 2) = central.x;
	TInv.at<double>(1, 2) = central.y;
}

template <class T>
T customPow(T _x, T _y)
{
	if (_x >= 0)
	{
		return std::pow(_x, _y);
	}
	else
	{
		return -std::pow(-_x, _y);
	}
}