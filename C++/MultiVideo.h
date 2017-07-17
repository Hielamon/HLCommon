#pragma once

#include "FrameArrayProcessor.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <chrono>
#include <commonMacro.h>

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

	void setSynchronizePoints(const std::vector<uint> &vFrameID, bool resetZero = false)
	{
		vBeginFrameID = vFrameID;
		if (resetZero)
		{
			uint minID = std::numeric_limits<unsigned int>::max();
			std::for_each(vBeginFrameID.begin(), vBeginFrameID.end(), [&minID](uint &id) {
				if (id < minID)
				{
					minID = id;
				}
			});

			std::for_each(vBeginFrameID.begin(), vBeginFrameID.end(), [&minID](uint &id) {
				id -= minID;
			});
		}
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
		int count = 0, testCount = 0, delay = 1, testAmount = 1;
		
		start_t = std::chrono::high_resolution_clock::now();

		while (ReadNextFrames(vCurrentFrame))
		{
			count++;
			testCount++;

			pFramesProcessor->process(vCurrentFrame, vResultFrames);

			if (pFramesProcessor->getEsc())break;
			

			if (bShowResultFrames && !ShowResultFrames(vResultFrames))break;

			end_t = std::chrono::high_resolution_clock::now();
			cost = std::chrono::duration_cast<std::chrono::nanoseconds>(end_t - start_t);

			if (bShowTimeConsuming)
			{
				long long costCount = cost.count() / testCount;
				std::cout << count << ";  " << 1e9 / costCount << " fps" << ";  "
					<< costCount * 1e-6 << " ms" << ";  " << std::endl;
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
		HL_INTERVAL_START

		if (!vFrame.empty()) vFrame.clear();

		bool bSuccessOpen = true;

#ifdef _OPENMP
#pragma omp parallel for
#endif // _OPENMP
		for (int i = 0; i < videoNum; i++)
		{
			if (!vCapture[i].grab())
			{
				std::cerr << "Failed to grab the " << i << " index camera" << std::endl;
				bSuccessOpen = false;
			}
		}

		if (!bSuccessOpen)
		{
			return false;
		}

		std::vector<cv::Mat> vTempFrame(videoNum);

#ifdef _OPENMP
#pragma omp parallel for
#endif // _OPENMP
		for (int i = 0; i < videoNum; i++)
		{
			if (!vCapture[i].retrieve(vTempFrame[i]))
			{
				std::cerr << "Failed to retrieve the " << i << " index camera" << std::endl;
				bSuccessOpen = false;
			}
		}

		if (!bSuccessOpen)
		{
			return false;
		}

		vFrame = vTempFrame;
		//****//***//***//用于usb接口软转换，仅用于本程序
		//frame_array.push_back(temp_frame_array[0]);
		//frame_array.push_back(temp_frame_array[1]);
		//frame_array.push_back(temp_frame_array[2]);
		//****//***//***//用于usb接口软转换，仅用于本程序
		HL_INTERVAL_END
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
		HL_INTERVAL_START

		size_t length = vResult.size();
		for (size_t i = 0; i < length; i++)
		{
			resizeShow(vResultWindowNames[i], vResult[i], bShowFitToMonitor);
			if (cv::waitKey(1) == 27) return false;
		}
		
		if (length == 0 && cv::waitKey(1) == 27) return false;

		HL_INTERVAL_END
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



