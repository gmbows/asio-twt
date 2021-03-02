#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <pthread.h>
#include <conio.h>

#include "Common.h"

extern std::string command,cursor;

extern std::stringstream statement;

std::vector<std::string> convert_char_array(char**,int);

std::string operator *(const std::string &s, int len);

template <class T>
void print(T t) {
    std::string space = " ";
    pthread_mutex_lock(&printLock);
    statement << t;
    std::cout << "\r" << space*(cursor.size()+command.size()+1) << "\r" << std::flush;
    std::cout << statement.str() << std::endl;
    std::cout << cursor << command << std::flush;
    statement.str("");
    pthread_mutex_unlock(&printLock);
}

template <class T,class... Args>
void print(T t,Args... args) {
    pthread_mutex_lock(&printLock);
    statement << t;
    pthread_mutex_unlock(&printLock);
    print(args...);
}

template <typename K,class V>
bool contains(std::map<K,V> &m,K k) {
    if(m.find(k) == m.end()) {
        return false;
    } else {
        return true;
    }
}

template <typename K,class V>
bool clean_insert(std::map<K,V> &m,K k,V v) {
    if(m.find(k) == m.end()) {
        m.insert({k,v});
        return true;
    } else {
        return false;
    }
}

std::string read_command(const std::string&);

void clear_buffer(char*,int);

std::vector<std::string> split(const std::string&,char);

void get_and_tokenize_input(std::string&, std::vector<std::string>&);
void tokenize(const std::string &input,std::string &cmd, std::vector<std::string> &args);

size_t import_file(const std::string &filename,char*&);
bool export_file(const std::string &filename,char*,size_t size);