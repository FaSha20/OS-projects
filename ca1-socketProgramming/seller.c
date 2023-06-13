#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <signal.h>
#include <time.h>
#include <errno.h>

#define STDIN 0

char* substr(const char *src, int m, int n)
{
    int len = n - m;
    char *dest = (char*)malloc(sizeof(char) * (len + 1));
    strncpy(dest, (src + m), len);
    return dest;
}

void alarm_handler(int sig) {
    printf("Time is up!The offer is expired.\n");
}

int find_ad_index(char Sarr[10][255] , char* add, int arr_len){
    for(int i = 0; i < arr_len; i++){
        char* find = substr(Sarr[i], 5, strlen(Sarr[i]) - 5);
        if(strcmp(find , add) == 0)
            return i;   
    }
    return -1;
}

int check_element(int a[], int elem, int size){
    for (int i = 0; i < size; i++){
        if (a[i] == elem){
            return 1;
        }
    }
    return 0;
}

char* first_word(char a[]){
    int i = 0;
    for (char *p = strtok(a," "); p != NULL; p = strtok(NULL, " "))
    {   
        i++;
        if(i == 1) return p;
    }
}


int setupServer(int port) {
    struct sockaddr_in address;
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    
    listen(server_fd, 4);

    return server_fd;
}

int acceptClient(int server_fd) {
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*) &address_len);
    return client_fd;
}


int main(int argc, char const *argv[]) {
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(EXIT_FAILURE);
    }
    char buffer[1024] = {0};
    char buffer2[1024] = {0};
    char u_neg[50] = "Under negotiation ";
    char exp_[50] = "Expired ";
    char ret_[50] = "Return to wait ";
    

    int max_sd, new_socket;
    fd_set master_set, working_set;
    int udp_port = atoi(argv[1]);
    int tcp_port, tcp_sockfd;
    int sock, broadcast = 1, opt = 1;
    struct sockaddr_in bc_address;
    int agahi_set[1024] = {0};
    int agahi_index = 0;


    //UDP
    int udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(udp_sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(udp_sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(udp_port); 
    bc_address.sin_addr.s_addr = inet_addr("255.255.255.255");
    bind(udp_sockfd, (struct sockaddr *)&bc_address, sizeof(bc_address));
     

    while (1)
    {        
        FD_ZERO(&master_set);
        max_sd = STDIN;
        FD_SET(STDIN, &master_set);
        
        while (1) {
            working_set = master_set;
            select(max_sd + 1, &working_set, NULL, NULL, NULL);
            for (int i = 0; i <= max_sd; i++) {
                
                if (FD_ISSET(i, &working_set)) {
                    
                    if (i == STDIN){           // broadcast an advertisenmet
                        memset(buffer, 0, 1024);
                        int j = read(0, buffer, 1024);
                        
                        tcp_port =  atoi(substr(buffer, 0, 4));
                        size_t sz = sendto(udp_sockfd, buffer, strlen(buffer), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
                        
                        tcp_sockfd = setupServer(tcp_port); //TCP
                        agahi_set[agahi_index++] = tcp_sockfd;
                        FD_SET(tcp_sockfd, &master_set);
                        if (tcp_sockfd> max_sd)
                           max_sd = tcp_sockfd;
                    }

                    else if (check_element(agahi_set, i, agahi_index)){
                        
                        new_socket = acceptClient(i);
                        FD_SET(new_socket, &master_set);
                        if (new_socket > max_sd)
                            max_sd = new_socket;
                    }
                    else { // client sending msg
                        
                        char name[1024] = {0};
                        char buf[1024] = {0};
                        char offer[1024] = {0};
                        memset(buffer, 0, 1024);
                        memset(buffer2, 0, 1024);
                        recv(i , buffer, 1024, 0);
                        printf("A new offer is recived for: %s$\n", buffer);
                        strcpy(offer, buffer);
                        memset(name, 0, 1024);
                        memset(buf, 0, 1024);
                        strcpy(name, first_word(buffer));
                        strcpy(buffer2, u_neg);    
                        strcpy(buf, strcat(buffer2, name)); 
                        sendto(udp_sockfd, buf, strlen(buf), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));

                        signal(SIGALRM, alarm_handler);
                        siginterrupt(SIGALRM, 1);

                        write(1, "Do you accept it?(y/n)\n",24);
                        alarm(10);
                        memset(buffer2, 0, 1024);
                        int read_n = read(0, buffer2, 1024);
                        alarm(0);

                        if(read_n != -1){
                            if(buffer2[0] == 'y'){
                            memset(buffer, 0, 1024);
                            memset(buf, 0, 1024);
                            strcpy(buf, exp_);
                            strcpy(buffer, strcat(buf, name));
                            send(i, "Your offer has been accepted!", 30, 0);
                            int file_fd;
                            char* a = "\n";
                            //mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
                            //file_fd = open("file.txt", O_APPEND | O_RDWR | O_CREAT | O_TRUNC, mode);
                            file_fd = open("file.txt", O_APPEND | O_RDWR);
                            write(file_fd, strcat(offer, a), strlen(offer)+1);
                            close(file_fd);
                            }
                            else{
                                memset(buffer, 0, 1024);
                                memset(buf, 0, 1024);
                                strcpy(buf, ret_);
                                strcpy(buffer, strcat(buf, name));
                                send(i, "Your offer has been rejected!", 30, 0);
                            }
                        }
                        else{
                            memset(buffer, 0, 1024);
                            memset(buf, 0, 1024);
                            strcpy(buf, ret_);
                            strcpy(buffer, strcat(buf, name));
                            send(i, "It takes too long to respond...", 50, 0);
                        }
                        sendto(udp_sockfd, buffer, strlen(buffer), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));         
                            
                       
                        close(i);
                        FD_CLR(i, &master_set);
                        memset(buffer, 0, 1024);
                    }

                    
                }
                
            }

        }
    }
    return 0;
}