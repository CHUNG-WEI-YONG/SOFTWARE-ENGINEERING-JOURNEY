#include <iostream>
#include <thread>
#include <mutex>
using namespace std;

mutex mutex1;
mutex mutex2;
template<typename t>
t add(t x , t y){
    return x+y;
}


void task1(){
    lock_guard<mutex> lock1(mutex1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    cout<<"Lock A gain"<<endl;

    lock_guard<mutex> lock2(mutex2);
    cout<<"Gain two lock"<<endl;
}

void task2(){ lock_guard<mutex> lock1(mutex2);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    cout<<"Lock A gain"<<endl;

    lock_guard<mutex> lock2(mutex1);
    cout<<"Gain two lock"<<endl;}
int main(){
    thread t1(task1);
    thread t2(task2);
    t1.join();
    t2.join();

    return 0;
}