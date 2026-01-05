#include <iostream>
#include <stdlib.h>
#include <cctype>
#include <fstream>
#include <ostream>

void getin(int score);
void getin(std::string name , std::string lastname);

using namespace std;
int main(){
    ifstream fin;
    ofstream fout;
    fin.open("./score");
    fout.open("finalise");

    string firstname,lastname;
    int sum=0;
    int quiz=5;

    while(fin>>lastname>>firstname){
        sum=0;
        fout<<firstname<<" "<<lastname<<" ";
        for (int i=0;i<quiz;i++){
            int score;
            if (fin>>score){
                fout<<score<<" ";
                sum+=score;}
            else{
                fout<<0<<" ";   
            }
        }
        fout<<sum/quiz<<"\n";
    }


    
    fin.close();
    fout.close();

    return 0;

}


