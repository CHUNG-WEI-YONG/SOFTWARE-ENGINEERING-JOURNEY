#include <iostream>

void swap(int& i,int& j);
int partition(int array[],int low,int high);
void Quicksort(int array[],int low,int high);
void couting_sort(int array[],int n);
int main (){
   int num[]={2,56,7,34,5,12,6,8,56};
    int n=sizeof(num)/sizeof(num[0]);
    for (int i=1;i<n;i++){
        int key=num[i];
        if (num[i-1]<num[i]){continue;}
        int j=i-1;
        while(j>=0 &&num[j]>key){
            num[j+1]=num[j];
            j--;
        }
        num[j+1]=key; }



  


    

    for (int i=0;i<n;i++){
        std::cout<<num[i]<<" ";
    }

    return 0;
}
