#include <iostream>
#include <string>
using namespace std;

struct AdjNode{
    int id;
    AdjNode* next;
};

struct Q{
    AdjNode* bottom;
    AdjNode* top;
    int capacity;
};

AdjNode* create_node(int id){
    AdjNode* node=new AdjNode;
    node->id=id;
    node->next=nullptr;
    return node;
};

Q* create_queue(){
    Q* q=new Q;
    q->bottom=nullptr;
    q->top=nullptr;
    q->capacity=0;
    return q;
}

Q* add_member(Q* q,int id){
    AdjNode* n=create_node(id);
    if(q->top==nullptr){
        q->top=n;
    }
    if(q->bottom==nullptr){
    q->bottom=n;}
    else{
        q->bottom->next=n;
        q->bottom=n;
    }
    q->capacity++;
    return q;

}

AdjNode* pop(Q* q){
    AdjNode* top=q->top;
    q->top=q->top->next;
    q->capacity--;

    return top;
}

int top(Q* q){
    return q->top->id;
}

void show_all(Q* q){

    AdjNode* curr=q->top;
    int array[q->capacity];
    while(curr){
        cout<<"Next: "<<curr->id<<endl;
        curr=curr->next;
        
    }
    return;
}



