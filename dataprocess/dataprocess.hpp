#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include <cstdio>
#include <sstream>
#include "../config/config.hpp"
#include "../engine/aclengine.hpp"
class DataProcess{
private:
	int width;
	int height;
	FILE* pipe;
	static int maxIdx(float* arr,int size);
public:
	DataProcess()=default;
	DataProcess(std::string type,std::string outUrl,int fps,int width,int height);
	void process(std::vector<cv::Mat> &inputFrames,
		std::vector<int>& inputIdx,
		std::vector<cv::Mat>& originFrames,
		std::vector<int>& originIdx,
		std::vector<BufferInfo>& output);
	void writeFrame(std::vector<cv::Mat>& frames);
	void closeWriter();
};
