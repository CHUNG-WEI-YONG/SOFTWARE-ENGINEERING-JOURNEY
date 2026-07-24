#include <iostream>
#include <vector>

using namespace std;


struct maxHeap{
    private:
        vector<int>  heap;
        void shiftUp(int i){
            while(i>0&& heap[i]>heap[(i-1)/2]){
                int curr=heap[i];
                heap[i]=heap[(i-1)/2];
                heap[(i-1)/2]=curr;
                i=(i-1)/2;
            }
        }

        void shiftdown(int i){
            int n=heap.size();
            while((2*i+1)<n){
                int left=2*i+1;
                int right=2*i+2;
                int smallest=left;
                if(right<n&&heap[right]>heap[left]){
                    smallest=right;
                }

                if(heap[i]>=heap[smallest])break;
                swap(heap[i],heap[smallest]);
                i=smallest;

            }
        }

    public:
        void insert(int n){
            heap.push_back(n);
            shiftUp(heap.size()-1);

        }

        int pop(){
            if(heap.empty())return -1;
            int top=heap[0];
            heap[0]=heap.back();
            heap.pop_back();
            if(!heap.empty()){
            shiftdown(0);
            }
            return top;

        }

        bool empty(){
            return heap.empty();
        }
};

int main(){
    maxHeap a;
    for(int i=0;i<10;i++){
        a.insert(i);
    }
    a.insert(10);
    a.insert(25);
    a.insert(15);

    while(!a.empty()){
        int b=a.pop();
        cout<<b<<endl;
    }
}
