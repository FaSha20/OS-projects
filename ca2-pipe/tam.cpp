#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>
#include <sys/wait.h>
using namespace std;

int main()
{
    /*int fd[2];
    char buf[30];
    if(pipe(fd) == -1) {
    perror("ERROR creating a pipe\n");
    exit(1);
    }
    if(!fork()) {
    printf("CHILD: writing to the pipe\n");
    write(fd[1], "Hello mother (or father?)", 26);
    write(fd[1], "i am child", 26);
    printf("CHILD: exiting\n");
    exit(0);
    }
    else {
        printf("PARENT: reading from pipe\n");
        read(fd[0], buf, 26);
        printf("PARENT: read \"%s\"\n", buf);
        wait(NULL);
        printf("PARENT: exiting\n");
    }*/
    int a = atoi("we");
    
    a+=2;
    cout << a << endl;
    return(0);
}
