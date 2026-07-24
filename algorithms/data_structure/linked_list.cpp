#include <iostream>
using namespace std;

template <typename T>
class list{

};

template<typename T>
struct Node{
    T data;
    Node* prev;
    Node* next;
    Node(const T &value=T()):data(value),prev(nullptr),next(nullptr){};
};



template<typename T>
class iterator{
    public:
        using self_type=iterator<T>;
        using value_type=T;
        using reference= T&;
        using pointer=T*;
        using iterator_category=std::bidirectional_iterator_tag;
        using difference_type=ptrdiff_t;

    iterator(Node<T>*ptr=nullptr):node_ptr(ptr){};

    reference operator *() const {return node_ptr->data;}
    pointer operator ->()const{return &(node_ptr->data);}
    self_type & operator ++(){
        if(node_ptr){
            node_ptr=node_ptr->next;
        }
        return *this;
    }

    self_type operator ++(int){
        self_type temp=*this;
        ++(*this);
        return temp;
    }

    self_type & operator --(){
        if(node_ptr){
            node_ptr=node_ptr->prev;
        }
        return *this;
    }

    self_type operator --(int){
        self_type temp=*this;
        --(*this);
        return temp;
    }

    const bool operator ==(const self_type &other){
        return node_ptr==other.node_ptr;
    }

    const bool operator !=(const self_type &other){
        return node_ptr!=other.node_ptr;
    }

private:
    Node<T> *node_ptr;
    friend class list<T>;

};

int main(){
    return 0;
}




