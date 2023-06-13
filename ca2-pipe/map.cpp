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

#define SIZE 20
using namespace std;


map<string, int> make_key(char* addr, char* genrestr)
{
    map<string, int>genre_num;
	string line, word;
    vector<string> genres;

    char * token = strtok(genrestr, ",");
    while( token != NULL ) {
        string s = token;
        genres.push_back(s);
        token = strtok(NULL, ",");
    }
   
    fstream file(addr, ios::in);
	if(file.is_open())
	{
		while(getline(file, line))
		{
			stringstream str(line);
			while(getline(str, word, ',')){
                for(auto genre: genres){
                    if(word == genre){
                        genre_num[genre] ++;
                        break;
                    }
                }   
            }     
		}
	} 
    file.close();       
    return genre_num;
}

int main(int argc, char*argv[])
{
    char *addr = (char*)malloc(SIZE);
    int unnamed_pipe_fd = atoi(argv[0]);
    if(read(unnamed_pipe_fd, addr, SIZE) == -1) 
        printf("Error in reading\n");

    map<string, int> map = make_key(addr, argv[2]);
    
    string final = "";
    for(auto ele: map){
        final += ele.first + " " + to_string(ele.second) + " ";
    }    

    char key[1024]; 
    strcpy(key, final.c_str());
    int named_pipe_fd = atoi(argv[1]);
    write(named_pipe_fd, key, strlen(key));

    return 0;
}