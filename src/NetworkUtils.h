#pragma once
#include <tuple>
#include <iostream>
#include <asio.hpp>

#include "Common.h"
#include "Utility.h"

class TWT_Peer;

using asio::ip::tcp;

struct TWT_Packet {
    tcp::socket *sock;

    char *data;
    size_t size;

    void import_message(const std::string &message) {
        clear_buffer(this->data,this->size);
        for(int i=0;i<message.size();++i) {
            this->data[i] = message[i];
        }
    }

    TWT_Packet(tcp::socket *_sock,const std::string &_message): sock(_sock) {

        //Set buffer size to global default
        this->size = TWT_BUFFER_SIZE;

        //Allocate proper memory
        this->data = (char*)std::malloc(TWT_BUFFER_SIZE);

        this->import_message(_message);
    }
};

template <typename ThreadType>
struct TWT_ThreadPackage {
    std::tuple<TWT_Peer*,ThreadType*> package;

    TWT_ThreadPackage(TWT_Peer *peer,ThreadType *thread) {
        this->package = std::make_tuple(peer,thread);
    }
};