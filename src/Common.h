#pragma once
#include <asio.hpp>
#include <pthread.h>
#include <string>

#define TWT_BUFFER_SIZE 1024
#define TWT_PAD_TYPE 2
#define TWT_PAD_SIZE 64

//extern std::string command,cursor;

enum DataType {
    DATA_MSG,
    DATA_FILE,
};


extern pthread_mutex_t printLock;

extern int thread_count;

extern asio::io_context io_context;
