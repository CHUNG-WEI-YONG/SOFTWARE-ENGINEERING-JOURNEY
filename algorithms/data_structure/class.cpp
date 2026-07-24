#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <map>
#include <string>
using namespace std;
void compair(char p1,char p2,bool& r1, bool& r2);

int main(){
    char p1,p2;
    char ans;
    do{
        cout<<"Person 1:";
        cin>>p1;
        cout<<"Person 2:";
        cin>>p2;
        bool r1,r2;
        
        
        compair(p1,p2,r1,r2);

        if (r1){
            cout<<"First person win"<<endl;
        }
        else if (r2){
            cout<<"Second person win"<<endl;
        }

        else{
            cout<<"Same , no one win"<<endl;
        }

        cout<<"Continue?(Y/N)";
       
        cin>>ans;

    }while(ans!='n'&&ans!='N');   
}

void compair(char p1,char p2,bool& r1, bool& r2){
    if (p1==p2){
        r1=r2=false;
    }
    else if((p1=='P'||p1=='p')&&(p2=='r'||p2=='R')){
        r1=true;
        r2=false;

    }
    else if((p2=='P'||p2=='p')&&(p1=='r'||p1=='R')){
        r1=false;
        r2=true;
    }
    else if((p1=='r'||p1=='R')&&(p2=='s'||p2=='S')){
        r1=true;
        r2=false;

    }
    else if((p1=='s'||p1=='S')&&(p2=='r'||p2=='R')){
        r2=true;
        r1=false;
    }
    else if((p1=='S'||p1=='s')&&(p2=='P'||p2=='p')){
        r1=true;
        r2=false;

    }
    else if((p1=='P'||p1=='p')&&(p2=='S'||p2=='s')){
        r2=true;
        r1=false;

    }
}






