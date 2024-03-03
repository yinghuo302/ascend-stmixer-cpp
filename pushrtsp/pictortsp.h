#pragma once
#include <iostream>
#include <stdlib.h>
#include <sys/time.h>
#include <memory>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <opencv2/opencv.hpp>

extern"C" {
	#include <libavutil/mathematics.h>
	#include <libavutil/time.h>
	#include "libavcodec/avcodec.h"
	#include "libavformat/avformat.h"
	#include "libswscale/swscale.h"
	#include "libavutil/imgutils.h"
	#include "libavutil/opt.h"
};
class VideoWriter
{
public:
	VideoWriter();
	~VideoWriter();
	VideoWriter(std::string avFormat,std::string out_file,int height,int width,int nframe);
	void writeFrame(cv::Mat frame);
private:
	int AvInitAvInit(std::string avFormat,std::string out_file,int height,int width,int nframe);
	void YuvDataInit();
	void BgrDataInint();
	int YuvDataToRtsp(void *dataBuf, uint32_t size, uint32_t seq);
	int BgrDataToRtsp(void *dataBuf, uint32_t size, uint32_t seq);
	int FlushEncoder();
    AVFormatContext* g_fmtCtx;
    AVCodecContext* g_codecCtx;
    AVStream* g_avStream;
    AVCodec* g_codec;
    AVPacket* g_pkt;
	AVFrame* g_yuvFrame;
	uint8_t* g_yuvBuf;
	AVFrame* g_rgbFrame;
	uint8_t* g_brgBuf;
	int g_yuvSize;
	int g_rgbSize;
	struct SwsContext* g_imgCtx;
	bool g_bgrToRtspFlag;
	bool g_yuvToRtspFlag;
	uint32_t cnt;
};
