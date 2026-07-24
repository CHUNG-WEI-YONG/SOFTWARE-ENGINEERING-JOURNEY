#include <iostream>

void swap(int& i,int& j);
int partition(int array[],int low,int high);
void Quicksort(int array[],int low,int high);
void couting_sort(int array[],int n);
int main (){
   int num[]={2,56,7,34,5,12,6,8,56};
    int n=sizeof(num)/sizeof(num[0]);
    /*for (int i=1;i<n;i++){
        int key=num[i];
        if (num[i-1]<num[i]){continue;}
        int j=i-1;
        while(j>=0 &&num[j]>key){
            num[j+1]=num[j];
            j--;
        }
        num[j+1]=key; */


       // Quicksort(num,0,n-1);//
       counting_sort(num,n);


    

    for (int i=0;i<n;i++){
        std::cout<<num[i]<<" ";
    }

    return 0;
}


void counting_sort(int array[],int size){
    int max=array[0];
    for (int i=0;i<size;i++){
        if (array[i]>max){
            max=array[i];
        }
    }

    int count[max+1];
    for (int i=0;i<=max;i++){
        count[i]=0;


    }

    for (int i=0;i<size;i++){
        while (array[i]>0){
        count[array[i]]++;
        array[i]--;
    }
    int index=0;
    for (int j=0;j<=max;j++){
        while(count[j]>0){
            array[index++]=j;
            count[j]--;

        }
        
    }
    }
}