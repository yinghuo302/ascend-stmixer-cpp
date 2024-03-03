#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>
#include <thread>
class ThreadPool {
public:
    ThreadPool(int worker=-1);
	~ThreadPool();
    void submit(const std::function<void()>& job);
    void stop();
	void start();
private:
    void threadLoop();
    bool terminate = false;           
    std::mutex mutex; 
    std::condition_variable condition;
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> jobs;
};