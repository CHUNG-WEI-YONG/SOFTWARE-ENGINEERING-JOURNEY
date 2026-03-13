#include <iostream>
#include <string>
using namespace std;

class student{
    public:
    void register_student(int i, string n,int a){
        id=i;
        name=n;
        age=a;
    };
    void input_result(int s,char g){
        score=s;
        grade=g;
    };
    void output_student(){
        cout<<"name: "<<name<<endl;
        cout<<"age: "<<age<<endl<<"grade: "<<grade<<endl;

    };

    private:
    int id;
    string name;
    int age;
    char grade;
    float score;
    
};

struct Node{
    student* student;
    Node* next;

};

struct list{
    Node* start;
    Node* end;
    int num=0;
};

Node* create_node(student* s){
    Node* node=new Node;
    node->student=s;
    node->next=nullptr;
    return node;
}

list* create_list(){
    list* l=new list;
    l->start=nullptr;
    l->end=nullptr;
    l->num=0;
    return l;

}

list* add_student(list* l, student* s){
    Node* n=create_node(s);
    n->next=l->start;
    if (l->end==nullptr){
        l->end=n;
    }
    l->start=n;
    return l;

}

void system(list* student_list,int id, string name,int age,int result=0,char grade='-'){
    student* s=new student;
    s->register_student(id,name,age);
    s->input_result(result,grade);
    student_list=add_student(student_list,s);
}

void system_print(list* student_list){
    Node* curr=student_list->start;
    while(curr!=nullptr){
        curr->student->output_student();
        curr=curr->next;
    }
    return ;
}
int main(){
    list* student_list=new list;
    system(student_list,10000,"chung",18,99,'A');
    system(student_list,10001,"cg",18,99,'A');
    system(student_list,10002,"clairie",19,90,'A');
    system(student_list,10003,"christopher",18,199,'A');
    system_print(student_list);




    return 0;
}