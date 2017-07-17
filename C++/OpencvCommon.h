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

template <class T>
inline bool CheckInSize(const cv::Point_<T> &pt, const cv::Size &size)
{
	return pt.x >= 0 && pt.y >= 0 && pt.x <= (size.width - 1) && pt.y <= (size.height - 1);
}

cv::Scalar RandomColor()
{
	uchar b = rand() % 256;
	uchar g = rand() % 256;
	uchar r = rand() % 256;
	return cv::Scalar(b, g, r);
}

inline void resizeShow(const std::string &name, const cv::Mat &img, bool sizeToScreen)
{
	//space emptied : pixel
	int edgeWidth = std::min(HL_SCREEN_WIDTH, HL_SCREEN_HEIGHT) * 0.1;
	if (sizeToScreen)
	{
		double scale = 1.0;
		int w = img.cols;
		int h = img.rows;
		if (w > (HL_SCREEN_WIDTH - edgeWidth))
		{
			scale = (HL_SCREEN_WIDTH - edgeWidth) / double(w);
			w *= scale;
			h *= scale;
		}

		if (h > (HL_SCREEN_HEIGHT - edgeWidth))
		{
			scale = (HL_SCREEN_HEIGHT - edgeWidth) / double(h);
			w *= scale;
			h *= scale;
		}

		cv::Mat resizedImg;
		cv::resize(img, resizedImg, cv::Size(w, h));
		cv::imshow(name, resizedImg);
	}
	else
	{
		cv::imshow(name, img);
	}
}

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