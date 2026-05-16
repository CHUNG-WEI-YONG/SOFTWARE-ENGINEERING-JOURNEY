#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#define max_buf 1024
#define max_token_size 72
#define LSH_TOK_DELINE "\t\r\n\a"

void lsh_loop();
char* lsh_getline();
char* lsh_getline();
char** lsh_splitline(char *line);
int lsh_execute(char ** args);
int lsh_launch(char **args);



int main(int argc,char** argv){
    lsh_loop();
    return EXIT_SUCCESS;

}

void lsh_loop(){
    char* line;
    char** args;
    int status;

    do{
        printf("$");
        line=lsh_getline();
        args=lsh_splitline(line);
        status=lsh_execute(args);

    }while(status);

}

char* lsh_getline(){
    int buf_size=max_buf;
    char* buffer=(char*)malloc(sizeof(char*)*buf_size);
    int c;
    int position=0;

    if(!buffer){
        printf("An error in allocate buffer");
        exit(EXIT_FAILURE);
    }

    while(1){
        c=getchar();

        if(c==EOF||c=='\n'){
            buffer[position]='\0';
            return buffer;
        }
        else{
            buffer[position]=c;
            position++;
        }

        if(position>=buf_size){
            buf_size+=max_buf;
            buffer=(char*)realloc(buffer,buf_size);
        }

        if(!buffer){
            printf("Error occur");
            exit(EXIT_FAILURE);
        }
    }
}

char** lsh_splitline(char *line){
    int bufsize=max_token_size , i=0;
    char* token;
    char** tokens=(char**)malloc(sizeof(char*)*bufsize);

    if(!tokens){
        printf("Exit error");
        exit(EXIT_FAILURE);
    }

    token=strtok(line,LSH_TOK_DELINE);

    while(token!=NULL){
        tokens[i]=token;
        i++;

        if(i>=bufsize){
            bufsize+=max_token_size;
            tokens=(char**)realloc(tokens,bufsize);
        }

        if(!tokens){
            printf("Error in allocating tokens");
            exit(EXIT_FAILURE);
        }
        token=strtok(NULL,LSH_TOK_DELINE);
    }
    tokens[i]=NULL;
    return tokens;
}

int lsh_launch(char **args){
    pid_t pid ,wpid;
    int status;

    pid=fork();
    if(pid==0){
        if(execvp(args[0],args)==-1){
            perror("lsh");
        }
    exit(EXIT_FAILURE);
}
    else if (pid<0)
    {
        perror("lsh");

    }
    else{
        do{
            wpid=waitpid(pid , &status,WUNTRACED);

        }while(!WIFEXITED(status)&& !WIFSIGNALED(status));
    }        
    return 1;
}

int lsh_cd(char** args);
int lsh_help(char** args);
int lsh_exit(char** atgs);

const char *buildin_str[]={
    "cd",
    "help",
    "exit",
};

int(* builtin_func[])(char**)={
    &lsh_cd,
    &lsh_help,
    &lsh_exit,

};

int lsh_numbuiltin(){
    return sizeof(buildin_str)/sizeof(char*);
}


int lsh_cd(char** args){
    if(args[1]==NULL){
        printf("Error , no argument");

    }
    else{
        if(chdir(args[1])!=0){
            perror("Lsh");
        }
    }
    return 1;
}

int lsh_help(char ** args){
    printf("This is CHUNG's terminal\n");
    printf("Enter command with its arguments to run it\n");
    printf("The built in function includes :\n");

    for(int i=0;i<lsh_numbuiltin();i++){
        printf("%d : %s",i,buildin_str[i]);
    }

    printf("Use the main command for more information\n");

    return 1;
}

int lsh_exit(char** args){
    return 0;
}

int lsh_execute(char ** args){
    int i=0;
    if(args[0]==NULL){
        return 1;
    }
    for(int i=0;i<lsh_numbuiltin();i++){
        if(strcmp(args[0],buildin_str[i])==0){
            return (*builtin_func[i])(args);
        }
    }
    return lsh_launch(args);
}