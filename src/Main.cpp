#include <iostream>
#include <vector>
#include <conio.h>

#include "Network.h"
#include "NetworkUtils.h"
#include "Utility.h"
#include "Thread.h"
#include "Common.h"

#include "Command.h"

/*TODO
 * New TWT_Connection data structure {Socket,address}
 * DONE File opening
 * Send file by filename
 * Send directory by dirname
 * States for peers
 * */

int main(int argc, char** argv) {
    //two way transfer

    pthread_mutex_init(&printLock,NULL);

    //convert char** args to string vector
    std::vector<std::string> arg_vector = convert_char_array(argv,argc);

    //Initial peer
    TWT_Peer *peer = new TWT_Peer(5555,10);

    //File import test
    char *temp;
    size_t size = import_file("twt.exe",temp);

    std::string cmd;
    std::vector<std::string> args;

    std::string lastConnection = "0";

    while(true) {
        std::string input = read_command(cursor);
        tokenize(input,cmd,args);
        if(cmd == "q" or cmd == "quit") {
            print("Exiting");
            break;
        } else if(cmd == "serv") {
            peer->TWT_Listen();

            //Wait for thread to enter listening function
            while(!peer->listening) {}
        } else if(cmd == "connect") {
			CMD_Connect(peer,cmd,args);
        } else if(cmd == "send") {
            CMD_Send(peer,cmd,args);
        } else if(cmd == "list") {
            for (auto it = peer->addressMap.begin(); it != peer->addressMap.end(); it++) {
                print(it->first, ": ", get_address(it->second));
            }
        } else if(cmd == "close") {
            CMD_Close(peer,cmd,args);
        } else {
            print("Unknown command:",cmd);
        }
    }

    return 0;
}
