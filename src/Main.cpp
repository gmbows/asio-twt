#include <iostream>
#include <vector>
#include <conio.h>

#include "Network.h"
#include "NetworkUtils.h"
#include "Utility.h"
#include "Thread.h"
#include "Common.h"

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
    TWT_Peer peer = TWT_Peer(5555,10);

    //File import test
    char *temp;
    size_t size = import_file("twt.exe",temp);

    std::string cmd;
    std::vector<std::string> args;

    std::string cursor = ">>";

    std::string lastConnection = "0";

    while(true) {
        std::string input = read_command(cursor);
        tokenize(input,cmd,args);
        if(cmd == "q" or cmd == "quit") {
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
                peer.TWT_Connect("127.0.0.1");
			}
        } else if(cmd == "send") {
            std::string msg = "Chat room opened";
            std::string sock;
            try {
                sock = args.at(1);
            } catch(const std::exception &e) {
                sock = lastConnection;
            }
            cursor = "send "+sock+">";
            while(msg != "q" and msg != "quit") {
                peer.TWT_FormatAndSend("(Remote) "+msg,sock);
                msg = read_command(cursor);
            }
            peer.TWT_FormatAndSend("(Remote) Chat room closed",sock);
            cursor = ">>";
        } else if(cmd == "list") {
            for (auto it = peer.addressMap.begin(); it != peer.addressMap.end(); it++) {
                print(it->first, ": ", get_address(it->second));
            }
        } else if(cmd == "close") {
            std::string sock_id;
            try {
                sock_id = args.at(1);
            } catch(const std::exception &e) {
//                print("Usage: ")
                sock_id = lastConnection;
            }
            if(!contains(peer.addressMap,sock_id)) {
                print("No active connection with id ",sock_id);
                continue;
            }
            for (auto it = peer.addressMap.begin(); it != peer.addressMap.end(); it++) {
                if(it->first == sock_id) {
                    asio::error_code err;
                    peer.TWT_CloseConnection(it->first);
                    print("Closed connection to ",get_address(it->second));
                    break;
                }
            }
        } else {
                print("Unknown command:",cmd);
        }
    }

    return 0;
}
