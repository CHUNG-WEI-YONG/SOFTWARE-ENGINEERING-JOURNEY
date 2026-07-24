#include <iostream>
struct Node{
    int data;
    Node* prev;
    Node* next;

};

Node* insertAtTail(Node* head,int num);
Node* insertAtHead(Node* head,int num);
void print(Node* head);
void reversed_print(Node* head);

int main(){
    Node* head=nullptr;
    head=insertAtTail(head,1);
    head=insertAtTail(head,2);
    head=insertAtTail(head,3);
    head=insertAtTail(head,4);
    head=insertAtTail(head,5);
    head=insertAtHead(head,6);
    head=insertAtTail(head,0);
    print(head);
    std::cout<<"\n";
    reversed_print(head);



}
void print(Node* head){
    Node* curr=head;
    while (curr!=nullptr){
        std::cout<<curr->data<<" ";
        curr=curr->next;
    }
}

Node* insertAtTail(Node* head,int num){
    if (head == nullptr){
        Node* node = new Node;
        node->data = num;
        node->prev = nullptr;
        node->next = nullptr;
        return node;
    }
    Node* curr =head;
    while (curr->next!=nullptr){
        curr=curr->next;
    }
    Node* temp=new Node;
    temp->data=num;
    temp->prev=curr;
    temp->next=nullptr;
    curr->next=temp;
    return head;

}

Node* insertAtHead(Node* head,int num){
    if (head == nullptr) {
    Node* node = new Node;
    node->data = num;
    node->prev = nullptr;
    node->next = nullptr;
    return node;
}
    Node* curr=head;
    Node* newnode=new Node;
    newnode->data=num;
    newnode->prev=nullptr;
    curr->prev=newnode;
    newnode->next=curr;
    head=newnode;

    return head;

}

void reversed_print(Node* head){
    Node* curr=head;
    while(curr->next!=nullptr){
        curr=curr->next;
    }
    while (curr!=nullptr){
        std::cout<<curr->data<<" ";
        curr=curr->prev;
    }
}

