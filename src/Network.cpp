#include "Network.h"
#include <asio.hpp>
#include <string>
#include <iostream>

#include "Utility.h"
#include "Common.h"

using asio::ip::tcp;

void TWT_Peer::TWT_Send(const std::string &s) {
    asio::error_code error;
    asio::write(*this->send_socket,asio::buffer(s), error);
}

void TWT_Peer::TWT_HandleConnection() {
    print("Handling connection");
    char msg[1024];
    clear_buffer(msg,1024);
    asio::error_code error;
    size_t len = this->receive_socket->read_some(asio::buffer(msg,1024), error);
    while(!error) {
        std::string s(msg);
        print(s);
        clear_buffer(msg,1024);
        size_t len = this->receive_socket->read_some(asio::buffer(msg,1024), error);
    }
    print("Error:",error.message());
    print("Closed socket");
}

void TWT_Peer::TWT_Listen() {

    print("Listening on",this->port,"...");
    try {
        this->acceptor->accept(*this->receive_socket);
    } catch(const std::exception &e) {
        print(e.what());
        return;
    }

    this->TWT_HandleConnection();

}

bool TWT_Peer::TWT_Connect(const std::string &host) {
    print("Connecting to",host);
    try {
        tcp::resolver::results_type endpoints = this->resolver->resolve(host, std::to_string(this->port));
        asio::connect(*this->send_socket, endpoints);
    } catch(const std::exception &e) {
        print(e.what());
        return false;
    }
    print("Connected to",host);
    this->TWT_Send("Test message");
    print("Shutting down socket");
    if(this->send_socket->is_open()) {
        print("Socket is open");
    } else {
        print("Socket is closed");
    }
    asio::error_code error;
    this->send_socket->shutdown(tcp::socket::shutdown_send,error);
    print(error.message());
    return true;

}
