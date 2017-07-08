#pragma once

#include "OpencvCommon.h"

class FrameArrayProcessor
{
public:
	FrameArrayProcessor() : mbEsc(false) {}

	bool getEsc()
	{
		return mbEsc;
	}

	void setEsc(bool esc)
	{
		mbEsc = esc;
	}
	
	virtual void process(const std::vector<cv::Mat> &srcs, std::vector<cv::Mat> &dsts) = 0;

private:
	bool mbEsc;
};

class OriginFrameArray : public FrameArrayProcessor
{
public:
	virtual void process(const std::vector<cv::Mat> &srcs, std::vector<cv::Mat> &dsts)
	{
		if (!dsts.empty())dsts.clear();
		//dst_array.resize(src_array.size());
		for (size_t i = 0; i < srcs.size(); i++)
		{
			cv::Mat temp;
			srcs[i].copyTo(temp);
			dsts.push_back(temp);
		}
	}
};
