#include <iostream>
#include <stdlib.h>
#include <iomanip>
using namespace std;

struct Node{
    char value;
    Node* next;
};

/*struct Stack{
    Node* head;
    int size;
};

Stack* create_stack(){
    Stack *s=new Stack;
    s->head=NULL;
    s->size=0;
    return s;
};

void push(Stack *s,char value){
    Node* Newnode=new Node;
    Newnode->value=value;
    Newnode->next=s->head;
    s->head=Newnode;
    s->size++;
}

char pop(Stack*s){
    if (s->size==0){
        return ' ';
    }
    char popvalue=s->head->value;
    Node* popnode=s->head;
    s->head=s->head->next;
    s->size--;
    delete popnode;
    return popvalue;
}

char peak(Stack *s){
    if (s->size==0){
        return ' ';
    }
    return s->head->value;
}

bool empty(Stack *s){
    return s->size==0;

}
int stack_Size(Stack *s){
    return s->size;
}

void delete_stack(Stack* s){
    while (!s){
        pop(s);
    }
    delete s;
}


int main(){
    Stack* a=create_stack();
    push(a,'5');
    push(a,'g');
    push(a,'h');
    char first=pop(a);
    cout<<first<<endl;
    cout<<stack_Size(a);
    cout<<empty(a);

    delete_stack(a);
    return 0;


}*/

