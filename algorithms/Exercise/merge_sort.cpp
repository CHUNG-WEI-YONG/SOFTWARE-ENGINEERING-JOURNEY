#include <iostream>

void merge_sort(int array[],int start,int end);
void merge(int array[],int start,int mid,int end);
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
       merge_sort(num,0,n-1);


    

    for (int i=0;i<n;i++){
        std::cout<<num[i]<<" ";
    }

    return 0;
}


void merge_sort(int array[],int start,int end){
    if(start<end){
        int mid=start+(end-start)/2;
        merge_sort(array,start,mid);
        merge_sort(array,mid+1,end);
        merge(array,start,mid,end);

    }
}


void merge(int array[],int start,int mid,int end) {
    int i,j,k;
    int n1=mid-start+1;
    int n2=end-mid;
    int left[n1],right[n2];

    for (int i=0;i<n1;i++){
        left[i]=array[start+i];
    }

    for (int j=0;j<n2;j++){
        right[j]=array[mid+1+j];
    }


    i=0;
    j=0;
    k=start;

    while (i<n1&&j<n2){
        if(left[i]<=right[j]){
            array[k]=left[i];
            i++;
        }
        else if(right[j]<left[i]){
            array[k]=right[j];
            j++;
        }
        k++;}

        while(i<n1){
            array[k]=left[i];
            i++;
            k++;

        }

        while(j<n2){
            array[k]=right[j];
            j++;
            k++;
        }

    
}