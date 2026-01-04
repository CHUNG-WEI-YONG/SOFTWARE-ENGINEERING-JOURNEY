#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <cctype>
using namespace std;

int main(){
    /*ifstream in_stream;
    in_stream.open("hello.exe");
    ofstream out_stream;
    out_stream.open("output.txt");
    out_stream<<"Hello , World"<<"This is my file";

    in_stream.close();
    out_stream.close();*/
    
    
    /*std::cout<<"Program start\n";
    ifstream fin;
    fin.open("./advise.txt");
    
    string line , lastline ="No sentence right now";
    if (fin){
        while (getline(fin,line)){
            if(!line.empty()){
                lastline=line;
            }
            
        }
        fin.close();
    }


    ofstream fout;
    fout.open("./advise.txt",ios::app);
    std::cout<<"Latest advise:\n";
    std::cout<<lastline<<"\n";
    cout<<"Enter your advise:\n";
    string userinput;
    getline(cin,userinput);
    fout<<userinput<<"\n";
    fout.close();*/
    ifstream int1,int2;
    int1.open("./sum");
    int2.open("./sum2");
    ofstream final;
    final.open("final.txt");
    int val1,val2;
    bool file1=static_cast<bool>(int1>>val1);
    bool file2=static_cast<bool>(int2>>val2);

    while (file1 && file2){
        if (val1<=val2){
            final<<val1<<endl;
            file1=static_cast<bool>(int1>>val1);
        }

        if (val2<val1){
            final<<val2<<endl;
            file2=static_cast<bool>(int2>>val2);
        }
    }

    while (file1){
        final<<val1<<endl;
        file1=static_cast<bool>(int1>>val1);
    }

    while (file2){
        final<<val2<<endl;
        file2=static_cast<bool>(int2>>val2);
    }
    int1.close();
    int2.close();
    final.close();
    
}



