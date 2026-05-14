#include <iostream>
#include<vector>
using namespace std;
#define buf_size 1024

int main(int argc,char** argv){
    lsh_loop();
    return EXIT_SUCCESS;
}

void lsh_loop(void){
    char* line;
    char** argv;
    int status;
    do{
        cout<<"$";
        line=lsh_getline();
        argv=lsh_split_line(line);
        status=lsh_execute(argv);

    }while(status);
}

vector<char> lsh_getline(void){

    int c;
    int bufsize=buf_size;
    int i=0;
    vector<char> buffer(bufsize,-1);

    while(1){
        c=getchar();

        if(c==EOF||c=='\n'){
            buffer[i]='\0';
            return buffer;
        }
        else{
            buffer[i]=c;
            i++;
        }

    }

}