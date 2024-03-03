#include "pool.hpp"
ThreadPool::ThreadPool(int worker) {
	if (worker==-1)
		worker = std::thread::hardware_concurrency(); // Max # of threads the system supports
    for (int ii = 0; ii < worker; ++ii) {
        threads.emplace_back(std::thread(&ThreadPool::threadLoop,this));
    }
}

void ThreadPool::start(){
	if(!terminate) return ;
	{
		std::unique_lock<std::mutex> lock(mutex);
		terminate = false;
	}
	condition.notify_all();
}

void ThreadPool::threadLoop() {
    while (true) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock, [this] {
                return !jobs.empty() || terminate;
            });
            if (terminate) {
				return;
            }
            job = std::move(jobs.front());
            jobs.pop();
        }
        try {
            job();
        } catch (const std::exception& e) {
			printf("Exception in thread: %s",e.what());
        }
    }
}

void ThreadPool::submit(const std::function<void()>& job) {
    {
        std::unique_lock<std::mutex> lock(mutex);
        jobs.push(std::move(job));
    }
    condition.notify_one();
}

void ThreadPool::stop() {
    if(terminate) return ;
	{
        std::unique_lock<std::mutex> lock(mutex);
        terminate = true;
    }
    condition.notify_all();
    for (std::thread& active_thread : threads) {
        active_thread.join();
    }
}

ThreadPool::~ThreadPool(){
	stop();
}