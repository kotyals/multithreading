#include<iostream>
#include<thread>
#include<vector>
#include<functional>
#include<queue>
#include<condition_variable>
#include<mutex>
#include<future>
#include<type_traits>

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
    
    template <typename T, typename... Args >
    auto enqueue(T&& t,Args&&... arg) -> std::future<typename std::invoke_result<T,Args...>::type>{
        using ReturnType = typename std::invoke_result<T,Args...>::type;
        auto task = make_shared<packaged_task<ReturnType()>>(
            [func = forward<T>(t), ... captured_arg = forward<Args>(arg)]() mutable{
                return func(move(captured_arg)...);
            }
        );

        future<ReturnType> fut = task -> get_future();
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