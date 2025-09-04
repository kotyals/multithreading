#include"ThreadPool.h"

ThreadPool::ThreadPool(size_t s): numThreads(s){
    for(int i=0; i<numThreads; i++){
        workers.emplace_back([this](){
            //call woker here
            this -> worker();

        });
    }
}
ThreadPool::~ThreadPool(){
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    cv.notify_all();
    for(auto& worker : workers){
        worker.join();
    }
}

void ThreadPool::worker(){
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
int square(int n){
    return n*n;
}
int main(){
    ThreadPool pool(4);

    auto f1 = pool.enqueue([](int n){
        return n*n;
    },10); //sending lambda
    auto f2 = pool.enqueue(square,20); //sending function
    auto f3 = pool.enqueue([](int a, int b){
        return a+b;
    }, 5, 15); //sending lambda with multiple arguments

    auto f4 = pool.enqueue([]{
        cout<<"Just printing something from a thread!"<<endl;
    });// sending lambda with no arguments
    std::cout << "f1 result: " << f1.get() << std::endl;
    std::cout << "f2 result: " << f2.get() << std::endl;
    std::cout << "f3 result: " << f3.get() << std::endl;
    f4.get(); // no return value, just executing the lambda
    return 0;
}