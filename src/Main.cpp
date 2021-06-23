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
    // char *temp;
    // size_t size = import_file("twt.exe",temp);
//    print(size);

	TWT_InputThread input = TWT_InputThread();
	// input.start(peer);
	
	// int hi;
	
	// for(int i=0;i<10;i++) std::cin >> hi;
	// while(true){}

    while(true) {
        std::vector<std::string> args;
        std::string cmd;

        //Get input
        std::string input = read_command(cursor);

        //Tokenize
        int arg_len = tokenize(input,args);
        cmd = args.at(0);

        if(cmd == "q" or cmd == "quit") {
            for(auto &connection: peer->addressMap) {
                peer->TWT_MarkSocketForClosing(connection.second);
            }
            peer->TWT_Deactivate();
            break;
		} else if(cmd == "link") {
			CMD_Serv(peer,cmd,args);
			CMD_Connect(peer,cmd,args);
        } else if(cmd == "serv") {
            CMD_Serv(peer,cmd,args);
        } else if(cmd == "connect") {
			CMD_Connect(peer,cmd,args);
        } else if(cmd == "chat") {
            CMD_Chat(peer,cmd,args);
        } else if(cmd == "list") {
            for (auto it = peer->addressMap.begin(); it != peer->addressMap.end(); it++) {
                print(it->first, ": ", get_address(it->second));
            }
        } else if(cmd == "close") {
            CMD_Close(peer,cmd,args);
        } else if(cmd == "send") {
            CMD_Send(peer,cmd,args);
        } else {
            print("Unknown command:",cmd);
        }
    }
    std::cout << "Exiting" << std::flush;
    return 0;
}
