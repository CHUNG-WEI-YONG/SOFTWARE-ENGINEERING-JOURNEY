#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;

mutex m;
condition_variable cv;
bool ready =false;

void traffic_light(){
    unique_lock<mutex> lock(m);
    cv.wait(lock,[]{return ready;});
    ready=false;
    cout<<"Red"<<endl;
    cv.wait(lock,[]{return ready;});
    ready=false;
    cout<<"Yellow"<<endl;
    cv.wait(lock,[]{return ready;});
    ready=false;
    cout<<"green"<<endl;
}

int main(){
    thread t(traffic_light);
    for (int i=0;i<3;i++)
    {
        for(int i=0;i<=10;i++){
            cout<<i<<endl;
            this_thread::sleep_for(chrono::seconds(1));
        }
        { 
            lock_guard<mutex> lock(m);
            ready=true;
        }
         
        
        cv.notify_one();
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    t.join();
}