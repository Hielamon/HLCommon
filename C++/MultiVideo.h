#pragma once

#include "FrameArrayProcessor.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <chrono>
#include <getTimeofDay.h>

//It's regretful that Multivideo cannot open 4 camera including the laptop's camera
//but it's uneffect for this program
class MultiVideo
{
public:
	MultiVideo() : videoNum(0), bShowTimeConsuming(false), bShowFitToMonitor(true), bShowResultFrames(true){}
	
	MultiVideo(const int cameraNum, std::shared_ptr<FrameArrayProcessor> & pProcessor) 
		: videoNum(cameraNum), bShowTimeConsuming(false), pFramesProcessor(pProcessor),
		bShowFitToMonitor(true), bShowResultFrames(true)
	{
		assert(cameraNum >= 0);
		std::stringstream io_str;
		if (videoNum > 0)vCapture.resize(videoNum);
		for (size_t i = 0; i < cameraNum; i++)
		{
			if (!vCapture[i].open(i + 1))
			{
				std::cerr << "Failed to open the " << i << " index camera" << std::endl;
				exit(-1);
			}
			else
			{
				io_str.str("");
				io_str << "camera " << i;
				vOriginWindowNames.push_back(io_str.str());
				io_str.str("");
				io_str << "camera " << i << " result ";
				vResultWindowNames.push_back(io_str.str());
			}
		}
	}

	MultiVideo(const std::vector<std::string> &vVideoFile, std::shared_ptr<FrameArrayProcessor> & pProcessor)
		: videoNum(vVideoFile.size()), bShowTimeConsuming(false), pFramesProcessor(pProcessor), 
		bShowFitToMonitor(true), bShowResultFrames(true)
	{
		std::stringstream io_str;
		assert(vVideoFile.size() >= 0);
		if (videoNum > 0)vCapture.resize(videoNum);
		for (size_t i = 0; i < videoNum; i++)
		{
			if (!vCapture[i].open(vVideoFile[i]))
			{
				std::cerr << "Failed to open the " << vVideoFile[i] << std::endl;
				exit(-1);
			}
			else
			{
				vOriginWindowNames.push_back(vVideoFile[i]);
				io_str.str("");
				io_str << vVideoFile[i] << " result ";
				vResultWindowNames.push_back(io_str.str());
			}
		}
	}
	
	//MultiVideo(const std::vector<std::vector<std::string>> &imagearray, FrameArrayProcessor * processor);

	~MultiVideo()
	{
		for (auto capture : vCapture)
			if (capture.isOpened())capture.release();
	}

	void setCamera(int cameraNum)
	{
		assert(cameraNum >= 0);

		if (videoNum > 0)
		{
			vCapture.clear();
			vOriginWindowNames.clear();
			vResultWindowNames.clear();
		}

		videoNum = cameraNum;
		if (videoNum > 0)vCapture.resize(videoNum);

		std::stringstream io_str;

		for (size_t i = 0; i < cameraNum; i++)
		{
			if (!vCapture[i].open(i + 1))
			{
				std::cerr << "Failed to open the " << i << " index camera" << std::endl;
				exit(-1);
			}
			else
			{
				io_str.str("");
				io_str << "camera " << i;
				vOriginWindowNames.push_back(io_str.str());
				io_str.str("");
				io_str << "camera " << i << " result ";
				vResultWindowNames.push_back(io_str.str());
			}
		}
	}

	void setSynchronizePoints(const std::vector<uint> &vFrameID)
	{
		vBeginFrameID = vFrameID;
	}

	void setVideoFile(const std::vector<std::string> &vVideoFile)
	{
		if (videoNum > 0)
		{
			vCapture.clear();
			vOriginWindowNames.clear();
			vResultWindowNames.clear();
		}

		videoNum = vVideoFile.size();
		if (videoNum > 0)vCapture.resize(videoNum);

		std::stringstream io_str;

		for (size_t i = 0; i < videoNum; i++)
		{
			if (!vCapture[i].open(vVideoFile[i]))
			{
				std::cerr << "Failed to open the " << vVideoFile[i] << std::endl;
				exit(-1);
			}
			else
			{
				vOriginWindowNames.push_back(vVideoFile[i]);
				io_str.str("");
				io_str << vVideoFile[i] << " result ";
				vResultWindowNames.push_back(io_str.str());
			}

		}
	}

	//void setImageArray(const std::vector<std::vector<std::string>> &imagearray);

	void setArrayProcessor(std::shared_ptr<FrameArrayProcessor> & pProcessor)
	{
		pFramesProcessor = pProcessor;
	}

	void setShowParameters(bool showTimeCost, bool showResultFrames, bool showFitToMonitor)
	{
		bShowTimeConsuming = showTimeCost;
		bShowResultFrames = showResultFrames;
		bShowFitToMonitor = showFitToMonitor;
	}
	
	virtual void run()
	{
		assert(pFramesProcessor && CheckValidCaputurer());

		cv::Mat trashImg;

		if (!vBeginFrameID.empty())
		{
			if (vBeginFrameID.size() > videoNum)
			{
				std::cerr << "vBeginFrameID's size " << vBeginFrameID.size() <<
					" is bigger than videoNum" << videoNum << std::endl;
				return;
			}

			for (size_t i = 0; i < vBeginFrameID.size(); i++)
			{
				for (size_t j = 0; j < vBeginFrameID[i]; j++)
				{
					if (!vCapture[i].read(trashImg))
					{
						std::cerr << "begin point is too large for " << i << "th image stream" << std::endl;
						return;
					}
				}
			}

			std::cout << "Synchronize Successfully" << std::endl;
		}

		std::chrono::time_point<std::chrono::steady_clock> start_t, end_t;
		std::chrono::nanoseconds cost;
		int count = 0, testCount = 0, delay = 1, testAmount = 200;
		
		start_t = std::chrono::high_resolution_clock::now();

		while (ReadNextFrames(vCurrentFrame))
		{
			count++;
			testCount++;

			pFramesProcessor->process(vCurrentFrame, vResultFrames);

			if (pFramesProcessor->getEsc())break;
			end_t = std::chrono::high_resolution_clock::now();
			cost = (std::chrono::duration_cast<std::chrono::nanoseconds>(end_t - start_t)) / testCount;

			if (bShowResultFrames && !ShowResultFrames(vResultFrames))break;

			if (bShowTimeConsuming)
			{
				std::cout << count << ";  " << 1e9 / cost.count() << " fps" << ";  "
					<< cost.count() * 1e-6 << " ms" << ";  " << std::endl;
			}
			

			if (count % testAmount == 0)
			{
				start_t = std::chrono::high_resolution_clock::now();
				testCount = 0;
			}
		}

		cv::destroyAllWindows();
		std::cout << "One MultiVideo Run Stop!" << std::endl;
	}

protected:
	bool ReadNextFrames(std::vector<cv::Mat> &vFrame)
	{
		if (!vFrame.empty()) vFrame.clear();

		std::vector<cv::Mat> vTempFrame;
		for (size_t i = 0; i < videoNum; i++)
		{
			if (!vCapture[i].grab())
			{
				std::cerr << "Failed to grab the " << i << " index camera" << std::endl;
				return false;
			}
		}

		for (size_t i = 0; i < videoNum; i++)
		{
			cv::Mat temp_frame;
			if (vCapture[i].retrieve(temp_frame))
				vTempFrame.push_back(temp_frame);
			else
			{
				vTempFrame.clear();
				std::cerr << "Failed to retrieve the " << i << " index camera" << std::endl;
				return false;
			}
		}
		vFrame = vTempFrame;
		//****//***//***//����usb�ӿ���ת���������ڱ�����
		//frame_array.push_back(temp_frame_array[0]);
		//frame_array.push_back(temp_frame_array[1]);
		//frame_array.push_back(temp_frame_array[2]);
		//****//***//***//����usb�ӿ���ת���������ڱ�����

		return true;
	}

	bool CheckValidCaputurer()
	{
		for (size_t i = 0; i < videoNum; i++)
		{
			if (!vCapture[i].isOpened())
			{
				return false;
			}
		}

		return true;
	}

	bool ShowResultFrames(const std::vector<cv::Mat> &vResult)
	{
		size_t length = vResult.size();
		for (size_t i = 0; i < length; i++)
		{
			resizeShow(vResultWindowNames[i], vResult[i], bShowFitToMonitor);
			if (cv::waitKey(1) == 27) return false;
		}
		
		if (length == 0 && cv::waitKey(1) == 27) return false;
		return true;
	}

	int ShowCurrentFrames()
	{
		if (vCurrentFrame.size() != videoNum)
		{
			std::cerr << vCurrentFrame.size() << " frames No enough with the video_num" << videoNum << std::endl;
			return 0;
		}
		for (size_t i = 0; i < videoNum; i++)
		{
			resizeShow(vOriginWindowNames[i], vCurrentFrame[i], bShowFitToMonitor);
			if (cv::waitKey(1) == 27) return 27;
		}
		return 0;
	}

	//capturer array
	std::vector<cv::VideoCapture> vCapture;
	
	//the frames are processing now
	std::vector<cv::Mat> vCurrentFrame;

	//the processed result frames
	std::vector<cv::Mat> vResultFrames;

	//windows' name array for showing the origin frames
	std::vector<std::string> vOriginWindowNames;

	//windows' name array for showing the result frames
	std::vector<std::string> vResultWindowNames;
	
	//the processor for input frames
	std::shared_ptr<FrameArrayProcessor> pFramesProcessor;

	//the count of videos
	int videoNum;

	//whether to show the time costed
	bool bShowTimeConsuming;

	//whether resize the image according to the monitor size
	bool bShowFitToMonitor;

	//whether show the result
	bool bShowResultFrames;

	//the synchronize points
	std::vector<uint> vBeginFrameID;
};



