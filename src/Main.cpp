#include <iostream>
#include <vector>

#include "Network.h"
#include "Utility.h"
#include "Thread.h"
#include "Common.h"

void* test(void *ptr) {
    print("Thread creation successful!");
    return nullptr;
}

int main(int argc, char** argv) {
    //two way transfer

    pthread_mutex_init(&printLock,NULL);

//    TWT_ThreadPool<TWT_SocketThread> pool = TWT_ThreadPool<TWT_SocketThread>(10,&test);
//    pool.start_threads();

    //convert char** args to string vector
    std::vector<std::string> arg_vector = convert_char_array(argv,argc);

    //Initial peer
    TWT_Peer peer = TWT_Peer(4422,10);

    std::string input;
    std::string cmd;
    std::vector<std::string> args;

    while(true) {
        std::cout << ">>" << std::flush;
        get_and_tokenize_input(cmd,args);
        if(cmd == "q") {
            print("Exiting");
            break;
        } else if(cmd == "serv") {
            peer.TWT_Listen();
        } else if(cmd == "connect") {
            peer.TWT_Connect("127.0.0.1");
        } else {
            print("Unknown command:",cmd);
        }
    }

    return 0;
}