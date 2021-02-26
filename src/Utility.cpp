#include "Utility.h"
#include <string>

//Converts char* array of length len to string vector
std::vector<std::string> convert_char_array(char** arr,int len) {
    std::vector<std::string> v;
    for(int i=0;i<len;++i) {
        std::string s(arr[i]);
        v.push_back(s);
    }
    return v;
}

void clear_buffer(char* buf,int len) {
    for(int i=0;i<len;++i) {
        buf[i] = '\0';
    }
}

std::vector<std::string> split(const std::string &s,const std::string &token) {
    std::vector<std::string> v;
    std::string run = "";
    for(auto &e : s) {
        if(std::to_string(e) == token) {
            if(run.size() == 0) continue;
            v.push_back(run);
            print(run);
            run = "";
        } else {
            run += e;
        }
    }
    v.push_back(run);
    return v;
}

void get_and_tokenize_input(std::string &cmd, std::vector<std::string> &args) {
    std::string input;
    std::getline(std::cin,input);
    if(split(input," ").size() == 0) return;
    cmd = split(input, " ").at(0);
    args = split(input, " ");
}