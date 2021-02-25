#include <iostream>
#include <vector>

#include "Network.h"
#include "Utility.h"

int main(int argc, char** argv) {
    //two way transfer

    //convert char** args to string vector
    std::vector<std::string> args = convert_char_array(argv,argc);

    TWT_Peer peer = TWT_Peer(13);
    if(args.size() >= 2) {
        peer.TWT_Listen();
    } else {
        peer.TWT_Connect("10.0.0.2");
    }
    return 0;
}