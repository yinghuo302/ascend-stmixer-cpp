#include "aclengine.hpp"
#include <cassert>
void AclEngine::initResource(int32_t deviceId){
	ret = aclInit(nullptr);
	ret = aclrtSetDevice(deviceId);
	ret = aclrtCreateContext(&context, deviceId);
}
void AclEngine::LoadModel(const char* modelPath){
	ret = aclmdlLoadFromFile(modelPath, &modelId);
	modelDesc =  aclmdlCreateDesc();
	ret = aclmdlGetDesc(modelDesc, modelId);
}
void AclEngine::allocateMem(){
	void* buffer;
	inputNum = aclmdlGetNumInputs(modelDesc);
	inputDataSet = aclmdlCreateDataset();
	for(int i=0;i<inputNum;i++){
		int size = aclmdlGetInputSizeByIndex(modelDesc,i);
		ret = aclrtMalloc(&buffer, size, ACL_MEM_MALLOC_HUGE_FIRST);
		aclDataBuffer *dataBuffer = aclCreateDataBuffer(buffer,size);
		input.push_back(BufferInfo(buffer,size));
		ret = aclmdlAddDatasetBuffer(inputDataSet, dataBuffer);
	}
	outputNum = aclmdlGetNumOutputs(modelDesc);
	outputDataSet = aclmdlCreateDataset();
	for(int i=0;i<outputNum;i++){
		int size = aclmdlGetOutputSizeByIndex(modelDesc,i);
		ret = aclrtMalloc(&buffer, size, ACL_MEM_MALLOC_HUGE_FIRST);
		aclDataBuffer *dataBuffer = aclCreateDataBuffer(buffer,size);
		output.push_back(BufferInfo(buffer,size));
		ret = aclmdlAddDatasetBuffer(outputDataSet, dataBuffer);
	}
}
void AclEngine::UnloadModel(){
	aclmdlDestroyDesc(modelDesc);
	aclmdlUnload(modelId);
}
void AclEngine::DestroyResource(){
	aclrtDestroyContext(context);
	ret = aclrtResetDevice(deviceId);
	aclFinalize();
}

void AclEngine::freeMem(){
	for(auto &bufferInfo: input)
		aclrtFree(bufferInfo.buffer);
	aclmdlDestroyDataset(inputDataSet);
	for(auto &bufferInfo: output)
		aclrtFree(bufferInfo.buffer);
	aclmdlDestroyDataset(outputDataSet);
}
AclEngine::AclEngine(int32_t deviceId,std::string model){
	printf("Start init resource\n");
	initResource(deviceId);
	LoadModel(model.c_str());
	allocateMem();
	printf("Init resource success\n");
}

void AclEngine::releaseResource(){
	printf("Start release resource\n");
	UnloadModel();
	freeMem();
	DestroyResource();
	printf("release resource success\n");
}
std::vector<BufferInfo>& AclEngine::inference(){
	std::unique_lock<std::mutex> lock(mutex);
	printf("Start Inference\n");
	ret = aclmdlExecute(modelId, inputDataSet, outputDataSet);
	printf("End Inference,ret:%d\n",(int)ret);
	
	std::vector<BufferInfo>* buffer = NULL;
	condition.wait(lock,[this]{
		return this->freeBufferHost > 0;
	});
	for(auto& buf: bufhost){
		if(buf[0].size==0){
			buffer = &buf;
			break;
		}
	}
	assert(buffer); assert((*buffer)[0].buffer); assert((*buffer)[1].buffer);
	freeBufferHost--;
	(*buffer)[0].size = 1;
	
	for(int i=0;i<outputNum;i++){
		aclrtMemcpy((*buffer)[i].buffer,output[i].size,output[i].buffer,output[i].size,ACL_MEMCPY_DEVICE_TO_HOST);
	}
	return *buffer;
}

void AclEngine::freeOutputHost(std::vector<BufferInfo>& outputHost){
	outputHost[0].size = 0;
	{
		std::unique_lock<std::mutex> lock(mutex);
		freeBufferHost++;
	}
	condition.notify_one();
}

void AclEngine::allocateBufferHost(int size){
	bufhost = std::vector<std::vector<BufferInfo>>(size,std::vector<BufferInfo>());
	void* bufferHost;
	for(int i=0;i<size;i++){
		for(int j=0;j<outputNum;j++){
			aclrtMallocHost(&bufferHost, output[j].size);
			bufhost[i].emplace_back(bufferHost,0);
		}
	}
	freeBufferHost = size;
}

void AclEngine::copyChannelToDevice(void* data,int chanId,int size){
	// char filename[100];
	// sprintf(filename,"./channel-%d.bin",chanId);
	// FILE* bin = fopen(filename,"wb");
	// fwrite(data,1,size,bin);
	// fclose(bin);
	printf("start copy data addr:%lld,chanId:%d,size:%d\n",(long long)data,chanId,size);
	aclrtMemcpy(input[0].buffer + chanId*size,size,data,size,ACL_MEMCPY_HOST_TO_DEVICE);
	printf("end copy data\n");
}

void AclEngine::test(){
	void* bufferHost;
	aclrtMallocHost(&bufferHost, input[0].size);
	aclrtMemcpy(bufferHost,input[0].size,input[0].buffer,input[0].size,ACL_MEMCPY_DEVICE_TO_HOST);
	aclrtFreeHost(bufferHost);
	FILE* bin = fopen("./test.bin","wb");
	fwrite(bufferHost,1,input[0].size,bin);
	fclose(bin);
}

void AclEngine::testOutput(std::vector<BufferInfo> & outputs){
	FILE* bin = fopen("./test_out.bin","wb");
	for(auto &output: outputs){
		fwrite(output.buffer,1,output.size,bin);
	}
	fclose(bin);
}

void AclEngine::setContext(){
	aclrtSetCurrentContext(context);
}