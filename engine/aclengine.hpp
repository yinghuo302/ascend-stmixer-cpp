#pragma once
#include "../config/config.hpp"
#include <acl/acl.h>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <cstdio>
class BufferInfo{
public:
	void* buffer;
	int size;
	BufferInfo() = default;
	BufferInfo(void* buffer,int size):buffer(buffer),size(size){}
};
class AclEngine{
	std::mutex mutex;
	std::condition_variable condition;
	int deviceId;
	uint32_t modelId;
	aclrtContext context;
	aclmdlDesc *modelDesc;
	aclmdlDataset *inputDataSet;	
	aclmdlDataset *outputDataSet;
	std::vector<BufferInfo> input;
	std::vector<BufferInfo> output;
	std::vector<std::vector<BufferInfo>> bufhost;
	int inputNum;
	int outputNum;
	aclError ret;
	void initResource(int32_t deviceId);
	void LoadModel(const char* modelPath);
	void allocateMem();
	void UnloadModel();
	void DestroyResource();
	void freeMem();
public:
	int freeBufferHost;
	AclEngine(int32_t deviceId,std::string model);
	void releaseResource();
	void allocateBufferHost(int size);
	std::vector<BufferInfo>& inference();
	void freeOutputHost(std::vector<BufferInfo>& outputHost);
	void copyChannelToDevice(void* data,int chanId,int size);
	void test();
	void testOutput(std::vector<BufferInfo> & outputs);
	void setContext();
};