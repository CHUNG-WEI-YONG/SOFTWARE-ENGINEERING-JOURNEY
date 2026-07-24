#include <iostream>
#include <stdlib.h>
using namespace std;
void read_and_clean(int& n);

int main(){
    int n;
    char ans;
    do {
        cout<<"Please enter an integer abd return:"<<endl;
        read_and_clean(n);
        cout<<"The strings convert to "<<n<<endl;
        cout<<"Again?(Y/N)";
        cin>>ans;
    }while(ans!='y'&&ans!='Y');

    return 0;
}

void read_and_clean(int& n){
    const int max_len=10;
    char digit[max_len];
    char next;
    cin.get(next);
    int index=0;
    while (next!='\n'){
        if (isdigit(next)&&index<max_len){
            digit[index]=next;
            index++;
        }
        cin.get(next);
    }

    digit[index]='\0';
    n=stoi(digit);
}