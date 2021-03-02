#pragma once

#include "Network.h"
#include "Common.h"
#include "Utility.h"

void CMD_Connect(TWT_Peer *peer, const std::string &cmd, const std::vector<std::string> &args) {
    try {
        peer->TWT_Connect(args.at(1));
    } catch(const std::exception &e) {
        peer->TWT_Connect("127.0.0.1");
    }
}

void CMD_Send(TWT_Peer *peer, const std::string &cmd, const std::vector<std::string> &args) {
    std::string msg = "Chat room opened";
    std::string sock = "0";
    try {
        sock = args.at(1);
    } catch(const std::exception &e) {
        sock = "0";
    }
    cursor = "send "+sock+">";
    while(msg != "q" and msg != "quit") {
        peer->TWT_FormatAndSend("(Remote) "+msg,sock);
        msg = read_command(cursor);
    }
    peer->TWT_FormatAndSend("(Remote) Chat room closed",sock);
    cursor = ">>";
}

void CMD_Close(TWT_Peer *peer, const std::string &cmd, const std::vector<std::string> &args) {
    std::string sock_id;
    try {
        sock_id = args.at(1);
    } catch(const std::exception &e) {
//                print("Usage: ")
        sock_id = "0";
    }
    if(!contains(peer->addressMap,sock_id)) {
        print("No active connection with id ",sock_id);
        return;
    }
    for (auto it = peer->addressMap.begin(); it != peer->addressMap.end(); it++) {
        if(it->first == sock_id) {
            asio::error_code err;
            peer->TWT_CloseConnection(it->first);
//            print("Closed connection to ",get_address(it->second));
            break;
        }
    }
}