#include<iostream>
#include<thread>
#include<vector>
#include<functional>
#include<queue>
#include<condition_variable>
#include<mutex>
#include<future>

using namespace std;

class ThreadPool {
private:
    size_t numThreads;
    bool stop = false;
    vector<thread> workers;
    queue<std::function<void()>> tasks;
    mutex queueMutex;
    condition_variable cv;
    
    void worker();
    
public:
    ThreadPool(size_t s);
    ~ThreadPool();
    
    template <typename T>
    std::future<int> enqueue(T&& t,int n){
        auto task = make_shared<packaged_task<int()>>(
            [func = forward<T>(t), n]() mutable{
                return func(n);
            }
        );

        future<int> fut = task -> get_future();
        {
            unique_lock<mutex> lock(queueMutex);
            tasks.emplace([task]() {
            (*task)();
        });
        }
        cv.notify_one();
        return fut;
    }

};