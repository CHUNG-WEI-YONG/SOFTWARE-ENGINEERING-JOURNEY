#include <iostream>
using namespace std;


struct controllBlock{
    int ref_count;
    controllBlock():ref_count(1) {};
};

template <typename T> 
class simpleSharePtr{
    private:
        T* ptr;
        controllBlock* control;

    public:
        simpleSharePtr():ptr(nullptr),control(nullptr){};

        explicit simpleSharePtr(T *p):ptr(p){
            if(p){
                control= new controllBlock;
            }
            else{
                control=nullptr;
            }
        };

        void release(){
            if(control){
                control->ref_count--;
                if(control->ref_count==0){
                    delete ptr;
                    ptr=nullptr;
                    delete control;
                    control=nullptr;
                }
            }
        };

        ~simpleSharePtr(){
                release();
        };

        simpleSharePtr& operator =(const simpleSharePtr& other){
            if(this!=&other){
                release();
                this.ptr=other.ptr;
                this.control=other.control;
                this.control->ref_count++;
            }
            return *this;
            
        } ;

        simpleSharePtr (const simpleSharePtr &s):ptr(s.ptr),control(s.control){
            if(control){
                control->ref_count++;
            }
        };

        simpleSharePtr( simpleSharePtr&& other):ptr(other.ptr),control(other.control){
            other.control=nullptr;
            other.ptr=nullptr;
        };

        simpleSharePtr& operator =(simpleSharePtr && other)noexcept{
            if(this!=&other){
                release();
                this->ptr=other->ptr;
                this->control=other->control;
                other.control=nullptr;
                other.ptr=nullptr;
            }
            return *this;
        };

        T* operator ->(){
            return ptr;
        };

        T& operator *(){
            return *ptr;
        };

        T* get()const{
            return ptr;
        };

        int user_count() const{
            return control? control->ref_count :0;
        };

        void reset(T* p=nullptr){
            release();
            ptr=p;
            if(p){
                control=new controllBlock();
            }

        };


};

struct student{
    int age;
    int id;

    student(int a,int b):age(a),id(b){};
};

int main(){
    simpleSharePtr<student> p_student;
    cout<<p_student.user_count()<<endl;
    simpleSharePtr<student> p2(new student(12,1));
    cout<<p2.user_count();
    simpleSharePtr<student> p3=p2;
    cout<<p3.user_count();
    return 0;
}