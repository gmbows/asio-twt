#include <asio.hpp>

pthread_mutex_t printLock;
int thread_count = 0;
asio::io_context io_context;