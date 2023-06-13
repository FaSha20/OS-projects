#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>


#define POSITION 14
#define FIFO_ADDRESS "/tmp/myfifo"
#define SIZE 1024
#define READ 0
#define WRITE 1


int count_file(const char* direction)
{
    int file_count = 0;
    DIR * dirp;
    struct dirent * entry;
    dirp = opendir(direction); 
    while ((entry = readdir(dirp)) != NULL){
        if (entry->d_type == DT_REG)
            file_count++;
    }
    closedir(dirp);
    return file_count;
}


int main(int argc, char* argv[]){
    char genrestr[SIZE];
    memset(genrestr, '\0', SIZE);

    int fp = open("./library/genres.csv", O_RDONLY);
    read(fp, genrestr, SIZE);

    char address[SIZE];
    strcpy(address, argv[1]);
    int fcount = count_file(address);
    
    //unnamed pipe
    int fd[2];
    if(pipe(fd) == -1)
        printf("An error occured!\n");
    
    //named pipe
    int fifo = mkfifo(FIFO_ADDRESS, 0777);
    int pipe_fd = open(FIFO_ADDRESS, O_RDWR);
    

    //Map
    char addr[]= "./library/part1.csv";
    for(int i = 1; i < fcount; i++){
        pid_t id = fork();
        if(id == 0)
        {
            char unnamed_pipe[2], named_pipe[3];
            sprintf(unnamed_pipe, "%d", fd[READ]);
            sprintf(named_pipe, "%d", pipe_fd);
            char *args[]={unnamed_pipe, named_pipe,genrestr, NULL};
		    execv("./map.out", args);
        }
        else
        {
            addr[POSITION] = i + '0';
            if(write(fd[WRITE], addr, sizeof(addr)) == -1)
                printf("Error in writing: %s\n", strerror(errno));                
            wait(NULL);
        }   
        close(id); 
    }
    pid_t child_pid;
    int s = 0;
    while ((child_pid = wait(&s)) > 0);


    //Reduce
    pid_t id2 = fork();
    if(id2 == 0)
    {
        char unnamed_pipe[2], named_pipe[3];
        sprintf(unnamed_pipe, "%d", fd[WRITE]);
        sprintf(named_pipe, "%d", pipe_fd);
        char *args[]={unnamed_pipe, named_pipe, NULL};
	    execvp("./reduce.out", args);
    }
    else
    {
        pid_t child_pid;
        int s = 0;
        while ((child_pid = wait(&s)) > 0);
    }   
    close(fd[WRITE]);

    
    //Write output in output.csv
    char text_[SIZE];
    memset(text_, '\0', SIZE);

    if(read(fd[READ], text_, SIZE) == -1) 
        printf("Error in reading\n");
    
    int output_fd = open("output.csv",  O_WRONLY | O_CREAT , 0644);
    if(write(output_fd, text_, strlen(text_)) == -1){
        printf("Error in writing %s\n", strerror(errno));
    }
    close(fd[READ]);
    close(pipe_fd);
    close(output_fd);
    return 0;
}