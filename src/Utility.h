#pragma once

#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> convert_char_array(char**,int);

template <class T>
void print(T t) {
    std::cout << t << std::endl;
}

template <class T,class... Args>
void print(T t,Args... args) {
    std::cout << t << " ";
    print(args...);
}

void clear_buffer(char*,int);

std::vector<std::string> split(const std::string&,const std::string &);

void get_and_tokenize_input(std::string&, std::vector<std::string>&);