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
       Quicksort(num,0,n);


    

    for (int i=0;i<n;i++){
        std::cout<<num[i]<<" ";
    }

    return 0;
}

void Quicksort(int array[],int low,int high){
    if(low<high){
        int pivotindex=partition(array ,low, high);
        Quicksort(array ,low,pivotindex-1);
        Quicksort(array ,pivotindex+1,high);
    }
}


int partition(int array[],int low,int high){
    int pivot=array[high]; 
    int i=low-1;
    for (int j=low;j<high;j++){
        if (array[j]<pivot){
            i++;
            swap(array[i],array[j]);
        }
        
    }
    swap(array[i+1],array[high]);
    return i+1;
}

void swap(int& i,int& j){
    int temp=i;
    i=j;
    j=temp;
}


