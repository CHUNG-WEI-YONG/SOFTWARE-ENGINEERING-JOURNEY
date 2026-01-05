#include <iostream>
#include <array>

void swap(int&a,int&b);
int main()
{
    int arr[]={9,8,7,6,5,4,3,2,1,0};
    int size=sizeof(arr)/sizeof(arr[0]);

    //bubble sort
    for (int step=0;step<size-1;step++){
    for (int i=0;i<size-1-step;i++){
        int& a=arr[i];
        int& b=arr[i+1];
        if (arr[i]>arr[i+1]){
            swap(a,b);
        }

    }}


   
    
    for (int i=0;i<size;i++){
    std::cout<<arr[i];
    }


}


void swap(int&a , int&b){
    int temp =a;
    a=b;
    b=temp;
}