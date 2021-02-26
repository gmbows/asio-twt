#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <pthread.h>

#include "Common.h"

std::vector<std::string> convert_char_array(char**,int);

template <class T>
void print(T t) {
    pthread_mutex_lock(&printLock);
    std::cout << t << std::endl;
    pthread_mutex_unlock(&printLock);
}

template <class T,class... Args>
void print(T t,Args... args) {
    pthread_mutex_lock(&printLock);
    std::cout << t;
    pthread_mutex_unlock(&printLock);
    print(args...);
}

void clear_buffer(char*,int);

std::vector<std::string> split(const std::string&,const std::string &);

void get_and_tokenize_input(std::string&, std::vector<std::string>&);