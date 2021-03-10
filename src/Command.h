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

void CMD_Chat(TWT_Peer *peer, const std::string &cmd, const std::vector<std::string> &args) {
    std::string msg = "Chat room opened";
    std::string sock = "0";
    try {
        sock = args.at(1);
    } catch(const std::exception &e) {
        sock = "0";
    }
    cursor = "chat "+sock+">";
    while(msg != "q" and msg != "quit") {
        peer->TWT_FormatAndSend(msg,sock);
        msg = read_command(cursor);
    }
    peer->TWT_FormatAndSend("Chat room closed",sock);
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
    peer->TWT_MarkSocketForClosing(sock_id);
}