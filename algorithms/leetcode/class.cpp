#include <iostream>
using namespace std;
int main(){
    int sum=0;
    int count=0;
    cout<<"Enter time you want:";
    cin>>count;
    for(int i=1;i<=count;i++){
        int num;
        cout<<"Please enter your number:";
        cin>>num;
        sum+=num;
        int avg=0;
        avg=sum/i;
        cout<<avg<<endl;
    }
    cout<<sum/count;
    return 0;


}