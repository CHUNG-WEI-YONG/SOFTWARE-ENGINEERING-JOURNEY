#include <iostream>
#include <stack>
#include <string>
using namespace std;


int precedence(char op){
    if(op=='+'||op=='-') return 1;
    else if(op=='*'||op=='/') return 2;
    else return 0;
}

void applyOp(stack <double> &values , char op){
    double v2=values.top();values.pop();
    double v1=values.top();values.pop();

    switch(op){
        case '+' :values.push(v1+v2);break;
        case '-' :values.push(v1-v2);break;
        case '*' :values.push(v1*v2);break;
        case '/' :values.push(v1/v2);break;

    }
}

double calculate (string token){
    bool negative=true;
    stack<double> num;
    stack<char> op;

    for(int i=0;i<token.length();i++){
        if (token[i]==' ')continue;
        else if(isdigit(token[i])||token[i]=='.'){
            string temp="";
            while(i<token.length()&&(isdigit(token[i])||token[i]=='.')){
                temp+=token[i];
                i++;
            }
            num.push(stod(temp));
            i--;
            negative=false;



        }

        else if(token[i]=='('){
            op.push('(');
            negative=true;
        }
        else if(token[i]==')'){
            while(!op.empty()&&op.top()!='('){

                applyOp(num,op.top());
                op.pop();
            }
            if(!op.empty())op.pop();
            negative=false;

        }
        else{
            if(token[i]=='-'&&negative){
                num.push(0);
            }
            while(!op.empty()&&precedence(op.top())>=precedence(token[i]))
            {
                applyOp(num,op.top());
                op.pop();
            }
            op.push(token[i]);
            negative=true;
        }
    }


        while(!op.empty()){
            applyOp(num,op.top());
            op.pop();

        
    }
    return num.top();
}
int main(){
    char answer;
    cout<<"Programme start"<<endl;
    cout<<"___________________________"<<endl;
    do{
    cout<<"Enter your expression:";    
    string input;
    getline(cin,input);
    double result=calculate(input);
    cout<<"Print the result: "<<result<<endl;
    cout<<"Continue or not(y/N)"<<endl;
    cin>>answer;
    cin.ignore();}
    while(answer=='y'||answer=='Y');

    

    return 0;
}