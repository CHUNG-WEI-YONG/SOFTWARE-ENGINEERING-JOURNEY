#include <iostream>
using namespace std;


struct Node{
    int num;
    Node* next;
};
void print();
void add_node(int times);
void insert_node(int insert_value,int num);
void del(int position);
void reversed();
void reversed_recursion(Node* p);

Node* head;
int main(){
    head=nullptr;
    insert_node(1,1);
    insert_node(2,2);
    insert_node(3,3);
    insert_node(4,4);
    print();
    reversed_recursion(head);
    print();
}

void reversed(){
    Node* curr=head;
    Node* prev=nullptr;
    Node* future;
    while (curr!=nullptr){
        future=curr->next;
        curr->next=prev;
        prev=curr;
        curr=future;
    }
    head=prev;
}

void reversed_recursion(Node* p){
    if (p==nullptr||p->next==nullptr){
        head=p;
        return;
    }
    reversed_recursion(p->next);
    Node* curr=p->next;
    curr->next=p;
    p->next=nullptr;


}




void add_node(int times){
    Node* curr=head;
    while(curr->next!=nullptr){
        curr=curr->next;
    }
    for (int i=0;i<times;i++){
        Node* new_node=new Node;
        int val;
        cout<<"enter a number:";
        cin>>val;
        new_node->num=val;
        new_node->next=nullptr;
        curr->next=new_node;
        curr=new_node;
        print();
    }

}

void insert_node(int insert_value,int num){
    Node* curr=head;
    Node* insert=new Node;
    insert->num=insert_value;
    insert->next=nullptr;
    if (num==1){
        insert->next=head;
        head=insert;
        return ;
        
    }
    for (int i=0;i<num-2;i++){
        if (curr->next==nullptr){
            std::cout<<"Invalid index";
        }
        curr=curr->next;
    }
    insert->next=curr->next;
    curr->next=insert;

}
void print(){
    Node* curr=head;
    while (curr->next!=nullptr){
        std::cout<<curr->num<<" ";
        curr=curr->next;

    }
    std::cout<<curr->num<<endl;
}

void del(int position){
    Node* curr=head;
    if (position==1){
        head=curr->next;
        delete curr;
        return ;
    }
    Node* prev;
    for (int i=0;i<position-2;i++){
        curr=curr->next;
    }
    prev=curr;
    curr=curr->next;
    prev->next=curr->next;
    delete curr;


}