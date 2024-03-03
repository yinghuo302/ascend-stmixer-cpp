#include <cstdio>
#include <opencv2/opencv.hpp>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "config/config.hpp"
#include "engine/aclengine.hpp"
#include "dataprocess/dataprocess.hpp"
#include "thread/pool.hpp"
#include "common/common.h"
AclEngine* engine;
DataProcess* processor;
int current = 1;
std::mutex mutex; 
std::condition_variable condition;


void processFrame(cv::Mat frame,int idx){
	cv::resize(frame,frame,cv::Size(cfg::WEITH,cfg::HEIGHT));
	cv::cvtColor(frame,frame,cv::COLOR_BGR2RGB);
	frame.convertTo(frame,CV_32FC3,1./255.);
	cv::Mat channels[3];
	cv::split(frame, channels);
	engine->copyChannelToDevice(channels[0].data,idx,cfg::HEIGHT*cfg::WEITH*4);
	engine->copyChannelToDevice(channels[1].data,cfg::NFRAME+idx,cfg::HEIGHT*cfg::WEITH*4);
	engine->copyChannelToDevice(channels[2].data,cfg::NFRAME*2+idx ,cfg::HEIGHT*cfg::WEITH*4);
}

void process(std::vector<cv::Mat>& inputFrames,
	std::vector<int>& inputIdx,
	std::vector<cv::Mat>& originFrames,
	std::vector<int>& originIdx,int id)
{
	engine->setContext();
	LOG("start %d\n",id);
	for(int i=0;i<cfg::NFRAME;i++){
		processFrame(inputFrames[i],i);
	}
	std::vector<BufferInfo>& output = engine->inference();
	printf("start postprocess %d\n",id);
	processor->process(inputFrames,inputIdx,originFrames,originIdx,output);
	printf("End postproces %d\n",id);
	printf("start free output %d\n",id);
	engine->freeOutputHost(output);
	printf("end free output %d\n",id);
	{
        std::unique_lock<std::mutex> lock(mutex);
		condition.wait(lock,[=](){
			return current == id;
		});
    }
	processor->writeFrame(originFrames);
	{
		std::unique_lock<std::mutex> lock(mutex);
		current++;
	}
	condition.notify_all();
	printf("finish %d\n",id);
}

int main(){
	auto cap = cv::VideoCapture(cfg::IN);
	int fps = cap.get(cv::CAP_PROP_FPS); // 帧率
	int width = cap.get(cv::CAP_PROP_FRAME_WIDTH); // 视频帧宽度
	int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
	engine = new AclEngine(0,cfg::MODEL_FILE);
	processor = new DataProcess(cfg::OUT_TYPE,cfg::OUT,fps,width,height);
	engine->allocateBufferHost(5);
	std::vector<cv::Mat> inputFrames;
	std::vector<int> inputIdx;
	std::vector<cv::Mat> originFrames;
	std::vector<int> originIdx;
	int cnt = 0, idx = 0,c = 0;
	float sample = 0;
	ThreadPool pool(4);
	while(cap.isOpened()){
		cv::Mat frame;
		cap >> frame;
		if (frame.empty()||c==25) break;
		idx += 1;
		sample += 1;
		originFrames.push_back(frame);
		originIdx.push_back(idx);
		if(sample>=cfg::SAMPLE){
			sample -= cfg::SAMPLE;
			inputFrames.push_back(frame);
			inputIdx.push_back(idx);
			++cnt;
			if(cnt==cfg::NFRAME){
				cnt = 0;idx = 0; c++;
				printf("submit %d\n",c);
				// process(inputFrames,inputIdx,originFrames,originIdx,c);
				pool.submit([=]()mutable{
					process(inputFrames,inputIdx,originFrames,originIdx,c);
				});
				inputFrames.clear();
				inputIdx.clear();
				originFrames.clear();
				originIdx.clear();
			}
		}
		
	}
	engine->releaseResource();
	processor->closeWriter();
	return 0;
}

