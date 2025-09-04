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
    
    void worker(){
        while(true){
        std::function<void()> task;
            {
                unique_lock<mutex> lock(queueMutex);
                cv.wait(lock,[this]{ return stop || !tasks.empty(); });

                if( stop && tasks.empty()) return;

                task = std::move(tasks.front());
                tasks.pop();

            }
            task();
        }

    }
    
public:
    ThreadPool(size_t s): numThreads(s){
        for(int i=0; i<numThreads; i++){
            workers.emplace_back([this](){
                //call woker here   
                this -> worker();

            });
        }
    }
    ~ThreadPool(){
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        cv.notify_all();
        for(auto& worker : workers){
            worker.join();
        }


    }
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