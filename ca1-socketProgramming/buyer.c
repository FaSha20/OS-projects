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

int find_ad_index(char Sarr[10][255] , char* add, int arr_len){
    for(int i = 0; i < arr_len; i++){
        char* find = substr(Sarr[i], 5, strlen(Sarr[i]) - 5);
        if(strcmp(find , add) == 0)
            return i;   
    }
    return -1;
}

int connectServer(int port) {
    int fd;
    struct sockaddr_in server_address;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) { // checking for errors
        printf("Error in connecting to server\n");
    }
    return fd;
}

int get_broadcast(int port){
    int sock, broadcast = 1, opt = 1;
    char buffer[1024] = {0};
    struct sockaddr_in bc_address;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(port); 
    bc_address.sin_addr.s_addr = inet_addr("255.255.255.255");

    bind(sock, (struct sockaddr *)&bc_address, sizeof(bc_address));
    return sock;
}

int main(int argc, char const *argv[]) {
    int tcp_sockfd,udp_sockfd;
    char sent_ads[10][255]; 
    int sent_ads_index = 0;
    char buff[1024] = {0};
    char line[1024] = {0};
    char* ad = malloc(255 * sizeof(' '));
    fd_set current_sockets, master_set, working_set;;
    int udp_port = atoi(argv[1]);
    
    if (argc < 2) 
    {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(EXIT_FAILURE);
    }
    
    udp_sockfd = get_broadcast(udp_port);

    while (1) {
        
        FD_ZERO(&current_sockets);
        FD_SET(udp_sockfd, &current_sockets);
        FD_SET(STDIN, &current_sockets);  //standard input 

        int res = select(udp_sockfd + 1, &current_sockets, NULL, NULL, NULL);

        if (FD_ISSET(udp_sockfd, &current_sockets)) //something broadcasted on udp_sockfd
        {
            memset(buff, 0, 1024);
            recv(udp_sockfd, buff, 1024, 0);
            
            printf("%c\n", ' ');
            
            if(buff[0] == 'E'){ //Expired ...
                printf("Seller Said: %s\n", buff);
                ad = substr(buff, 8, strlen(buff));
                int index = find_ad_index(sent_ads, ad, sent_ads_index);
                for(int i = index; i < sent_ads_index - 1; i++){
                    strcpy(sent_ads[i], sent_ads[i + 1]);
                }            
                sent_ads_index--;
            }                   
            else if(buff[0] == 'U'){ //Under negotiation ...
                printf("Seller Said: %s\n", buff);
                ad = substr(buff, 18, strlen(buff));
                int index = find_ad_index(sent_ads, ad, sent_ads_index);            
                char* port = substr(sent_ads[index], 0, strlen(sent_ads[index]) - 5);
                strcpy(sent_ads[index], strcat(port, " NEGO"));           
            }
            else if(buff[0] == 'R'){ //Return to wait ...
                printf("Seller Said: %s\n", buff);
                ad = substr(buff, 15, strlen(buff));
                int index = find_ad_index(sent_ads, ad, sent_ads_index);            
                char* port = substr(sent_ads[index], 0, strlen(sent_ads[index]) - 5);
                strcpy(sent_ads[index], strcat(port, " WAIT"));           
            }
            else{ //a new advertisement recived
                buff[strlen(buff)-1] = '\0';
                strcpy(sent_ads[sent_ads_index], strcat(buff, " WAIT"));
                sent_ads_index++;
                
            }

            printf("%s\n", "------Current ads-------");
            for(int i = 0; i < sent_ads_index; i++)
                printf("%d- %s\n", i + 1, substr(sent_ads[i], 5, strlen(sent_ads[i])));
        }
        if (FD_ISSET(STDIN, &current_sockets)) {
            memset(buff, 0, 1024);
            read(0, buff, 1024);
            int index = atoi(substr(buff, 0, 1)) - 1;
            char* price = substr(buff, 2, strlen(buff) - 1);
            int tcp_port = atoi(substr(sent_ads[index], 0, 4));
            
            tcp_sockfd = connectServer(tcp_port);
            char* name = substr(sent_ads[index], 5, strlen(sent_ads[index]) - 4);
            char* b = strcat(name, price);
            send(tcp_sockfd, b, strlen(b), 0);

            
            
            memset(buff, 0, 1024);
            recv(tcp_sockfd , buff, 1024, 0);

            printf("Seller response: %s\n", buff);
            close(tcp_sockfd);
        }
    }


    return 0;
}