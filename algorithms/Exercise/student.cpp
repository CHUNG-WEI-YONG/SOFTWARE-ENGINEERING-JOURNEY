#include <iostream>
#include <string>
#include <vector>
using namespace std;

struct student{
    string name;
    int score;
};

struct teacher{
    string name;
    vector<student> s;
};

int main(){
    student a,b,c,d,e;
    a.name='a',a.score=40;
    b.name='b',b.score=70;
    c.name='c',c.score=80;
    d.name='d',d.score=100;
    e.name='e',e.score=0;

    teacher t;
    t.name="ME";
    t.s={a,b,c,d,e};

    cout<<t.name<<endl;
    for(int  i=0;i<5;i++){
       cout<<t.s[i].name<<" "<<t.s[i].score<<endl;
    }
}