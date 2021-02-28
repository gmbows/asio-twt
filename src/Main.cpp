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

    //convert char** args to string vector
    std::vector<std::string> arg_vector = convert_char_array(argv,argc);

    //Initial peer
    TWT_Peer peer = TWT_Peer(5555,10);

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

            //Wait for thread to enter listening function
            while(!peer.listening) {}

        } else if(cmd == "connect") {
			try {
            	peer.TWT_Connect(args.at(1));
			} catch(const std::exception &e) {
				print("Usage: connect [address]");
			}
        } else {
            print("Unknown command:",cmd);
        }
    }

    return 0;
}
