#pragma once
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <fstream>

static int rct_size = 3 * sizeof(int);

inline void saveMat(cv::Mat &src, std::fstream &fs)
{
	int type = src.type();
	int rows = src.rows, cols = src.cols;
	int rct[3] = { rows, cols, type };
	fs.write((char*)rct, rct_size);

	fs.write((char*)src.data, src.step[0] * rows);
}

inline void loadMat(cv::Mat &src, std::fstream &fs)
{
	int rct[3];
	fs.read((char*)rct, rct_size);
	src.create(rct[0], rct[1], rct[2]);
	fs.read((char*)src.data, src.step[0] * rct[0]);
}

template<class T>
void saveRect(cv::Rect_<T> &rect, std::fstream &fs)
{
	T rectPara[4] = { rect.x, rect.y, rect.width, rect.height };
	fs.write((char*)rectPara, 4 * sizeof(T));
}

template<class T>
void loadRect(cv::Rect_<T> &rect, std::fstream &fs)
{
	T rectPara[4];
	fs.read((char*)rectPara, 4 * sizeof(T));
	rect.x = rectPara[0];
	rect.y = rectPara[1];
	rect.width = rectPara[2];
	rect.height = rectPara[3];
}