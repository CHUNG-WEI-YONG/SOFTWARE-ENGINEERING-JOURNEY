#include <iostream>
#include <stdlib.h>
#include <sstream>
using namespace std;


class result{
    private:
    int score;
    char grade;
    public:
    result();

    

    friend istream& operator >>(istream& ins,result& result);
    friend ostream& operator <<(ostream&outs,result& result);
    
};
int main(){
    result a= result();
    cin>>a;
    cout<<a;

    return 0;
}


istream& operator >>(istream& ins,result& result){
    double quiz1,quiz2,test1,test2;
    ins>>quiz1>>quiz2>>test1>>test2;
    result.score=(quiz1+quiz2)/20*25+test1/100*25+test2/100*50;
    return ins;
}


ostream& operator <<(ostream& outs,result& result){
    if (result.score<60){
        result.grade='F';
    }

    else if(result.score>=60&&result.score<70){
        result.grade='D';
    }

    else if(result.score>=70&&result.score<80){
        result.grade='C';

    }

    else if (result.score>=80&&result.score<90){
        result.grade='B';
    }

    else {
        result.grade='A';
    }

    outs<<"The total mark is "<<result.score<<" Grade is "<<result.grade

;
    return outs;
}

result::result(){
    score=0;
    grade=0;
}