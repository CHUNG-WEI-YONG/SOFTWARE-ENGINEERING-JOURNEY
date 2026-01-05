#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cctype>
#include <vector>
using namespace std;


int main (){
    ifstream fin;
    fin.open("./word");
    ofstream fout;
    fout.open("word_count");
    char al;
    int count=0;

    while (fin>>al){
        if (isalpha(al)){
            count++;

        }
        else{
            if (count!=0){
                fout<<count<<" ";
            }
            count=0;

        }

    }
    if (count > 0) {
    fout << count;
}

    fin.close();
    fout.close();

    return 0;
}
