#include <iostream>
#include <math.h>
using namespace std;
const int max=10;

class production_graph{
    private:
    int production_array[10];
    int scale_array[10];
    int len;

    public:
    void set_len(int length);
    void input_graph();
    void scale();
    void output();
    


    };


int main(){
    char ans;
    do{
    production_graph a;
    a.set_len(7);
    a.input_graph();
    a.scale();
    a.output();
    cout<<"Do you want to stop?('y' or 'n')";

    cin>>ans;
    }while (ans!='y'&& ans!='Y');


    return 0;
}

void production_graph::set_len(int length){
    len=length;
}



void production_graph::input_graph(){
    std::cout<<"Please enter your data:";
        for (int i=0;i<len;i++){
            int num;
            cin>>num;
            production_array[i]=num;
        }
    }

void production_graph::scale(){
    for (int i=0;i<len;i++){
        double sc=production_array[i]/1000.0;
        scale_array[i]=floor(sc+0.5);
    }
    
}


void production_graph::output(){
    for (int i=0;i<len;i++){
        for (int j=0;j<scale_array[i];j++){
            std::cout<<"*";
        }

    std::cout<<"\n";
    }
}