#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>

#define SIZE 1024

using namespace std;

map<string,int> reduce_keys(char* genre_map){
    string word;
    stringstream str(genre_map);
    vector<string> genres;
    map<string,int>genre_num;
    while(getline(str, word, ' ')){
        genres.push_back(word);
    }  
    for(int i = 0; i < genres.size(); i += 2 ){
        genre_num[genres[i]] += atoi(genres[i+1].c_str());
    }
    return genre_num;
}    


int main(int argc, char*argv[])
{
    char text_[SIZE];
    memset(text_, '\0', SIZE);
    string final;
    
    int named_pipe_fd = atoi(argv[1]);
    if(read(named_pipe_fd, text_, SIZE) == -1) 
        printf("Error in reading \n");

    map<string,int>map = reduce_keys(text_);
    
    for(auto ele: map){
        final += ele.first + " : " + to_string(ele.second) + "\n";
    }    
    final += "\0";

    int unnamed_pipe_fd = atoi(argv[0]);
    write(unnamed_pipe_fd, final.c_str(), final.size());
    return 0;
}