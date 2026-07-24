#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
using namespace std;

int main(){
atomic<int> x(0);
x.store(10);
int v=x.load();
cout<<v;
}
