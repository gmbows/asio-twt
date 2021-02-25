#include "Network.h"
#include <asio.hpp>
#include <string>
#include <iostream>

#include "Utility.h"
#include "Common.h"

using asio::ip::tcp;

void TWT_Peer::TWT_HandleConnection() {
    print("Handling connection :)");
}

void TWT_Peer::TWT_Listen() {
    print("Listening on",this->port,"...");
    try {
        this->acceptor->accept(*this->socket);
    } catch(const std::exception &e) {
        print(e.what());
        return;
    }

    this->TWT_HandleConnection();
    try {
        this->socket->close();
    } catch(const std::exception &e) {
        print(e.what());
    }
}

bool TWT_Peer::TWT_Connect(const std::string &host) {
    print("Connecting to",host);
    try {
        tcp::resolver::results_type endpoints = this->resolver->resolve(host, "daytime");
        asio::connect(*this->socket, endpoints);
    } catch(const std::exception &e) {
        print(e.what());
        return false;
    }
    print("Connected to",host);
    return true;

}
