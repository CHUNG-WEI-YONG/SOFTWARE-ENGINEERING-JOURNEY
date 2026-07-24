#include <iostream>
int binary(int array[],int start,int end,int target);


int main (){
   int num[]={2,4,7,34,45,52,66,78,156};
   int n=sizeof(num)/sizeof(num[0]);
   int index=binary(num,0,n,52);

   std::cout<<"Found it at "<<index;



    



    return 0;
}


int binary(int array[],int start,int end,int target){
    while (start<=end){
    int midd=(start+end)/2;
    if (array[midd]!=target){
        if (array[midd]<target){
            start=midd+1;
        }

        else if(array[midd]>target){
            end=midd-1;

        }
    }
    else if(array[midd]==target) {
        return midd;
    }}

        return -1;
    
}
