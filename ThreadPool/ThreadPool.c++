#include"ThreadPool.h"


int square(int n){
    return n*n;
}
int main(){
    ThreadPool pool(4);

    auto f1 = pool.enqueue([](int n){
        return n*n;
    },10);
    auto f2 = pool.enqueue(square,20);
    std::cout << "f1 result: " << f1.get() << std::endl;
    std::cout << "f2 result: " << f2.get() << std::endl;
    return 0;
}