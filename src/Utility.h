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