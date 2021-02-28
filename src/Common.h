#pragma once
#include <asio.hpp>
#include <pthread.h>

#define TWT_BUFFER_SIZE 1024

extern pthread_mutex_t printLock;

extern int thread_count;

extern asio::io_context io_context;
