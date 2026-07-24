#include <iostream>

int main(){
    int array[]={9,8,7,6,5,4,3,2,1,0};

    for(int i=0;i<10-1;i++){
        for (int j=0;j<10-i-1;j++){
            if(array[j]>array[j+1]){
                int temp=array[j];
                array[j]=array[j+1];
                array[j+1]=temp;
            }
        }
    }
    for (int i=0;i<10;i++){
        std::cout<<array[i]<<std::endl;
    }
}