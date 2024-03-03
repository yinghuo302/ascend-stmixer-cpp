#include "dataprocess.hpp"
DataProcess::DataProcess(std::string type,std::string outUrl,int fps,int width,int height):width(width),height(height){
	char command[200];
	if(type=="rtsp")
		sprintf(command,"ffmpeg -y -f rawvideo -vcodec rawvideo -pix_fmt bgr24 -s %dx%d -r %d -i - -c:v libx264 -pix_fmt yuv420p -preset ultrafast -f rtsp %s",width,height,fps,outUrl.c_str());
	else if (type=="file")
		sprintf(command,"ffmpeg -y -f rawvideo -vcodec rawvideo -pix_fmt bgr24 -s %dx%d -r %d -i - -c:v libx264 %s",width,height,fps,outUrl.c_str());
	printf("%s\n",command);
	pipe = popen(command,"w");
}

struct Query{
	int idx;
	int label;
	Query()=default;
	Query(int idx,int label):idx(idx),label(label){}
};


void DataProcess::process(std::vector<cv::Mat> &inputFrames,
	std::vector<int>& inputIdx,
	std::vector<cv::Mat>& originFrames,
	std::vector<int>& originIdx,
	std::vector<BufferInfo>& output)
{
	int size = originFrames.size(),i_input = 0;
	std::vector<Query> validQuery;
	for(int query=0;query<cfg::NQUERY;query++){
		float* scores = ((float* )output[0].buffer) + query * cfg::NCLASS;
		int k = maxIdx(scores,cfg::NLABEL);
		if (scores[cfg::NLABEL]  > cfg::BG_THRESHOLD || scores[k] < cfg::THRESHOLD)
			continue;
		validQuery.emplace_back(query,k);
	}
	for(int i=0;i<size;i++){
		if (inputIdx[i_input] < originIdx[i])
			i_input++;
		for(auto &query: validQuery){
			float* scores = ((float* )output[0].buffer) + query.idx * cfg::NCLASS;
			float* boxes = ((float* )output[1].buffer) + i_input * cfg::NQUERY * 4 + query.idx * 4;
			int x1 = (boxes[0] * width) / cfg::WEITH;
			int x2 = (boxes[2] * width) / cfg::WEITH;
			int y1 = (boxes[1] * height) / cfg::HEIGHT;
			int y2 = (boxes[3] * height) / cfg::HEIGHT;
			cv::rectangle(originFrames[i],cv::Point(x1,y1),cv::Point(x2,y2),cv::Scalar(0,0,255));
			std::stringstream ss;
			ss << cfg::LABELS[query.label] << std::fixed << std::setprecision(2) << scores[query.label];
			cv::putText(originFrames[i], ss.str() ,cv::Point(x1,y1),cv::FONT_HERSHEY_COMPLEX,0.8,cv::Scalar(0,0,0));
		}
	}
}
void DataProcess::writeFrame(std::vector<cv::Mat>& frames){
	printf("start write frame\n");
	for(auto & frame: frames){
		// printf("test\n");
		// printf("frame info, addr:%lld, rows: %d,cols:%d\n",(long long)frame.data,frame.rows,frame.cols);
		fwrite(frame.data,1,frame.rows*frame.cols*3,pipe);
	}
	printf("end write frame\n");
}

void DataProcess::closeWriter(){
	fclose(pipe);
}
int DataProcess::maxIdx(float* arr,int size){
	int ret = 0;
	for(int i=1;i<size;i++){
		if(arr[i]>arr[ret])
			ret = i;
	}
	return ret;
}

