#include "Network.h"
#include <asio.hpp>
#include <string>
#include <queue>
#include <iostream>

#include "Utility.h"
#include "Common.h"
#include "NetworkUtils.h"

using asio::ip::tcp;

tcp::socket* TWT_Peer::TWT_SafePop(std::queue<tcp::socket*> *queue) {
    pthread_mutex_lock(&this->popLock);
    tcp::socket *sock;
    try {
        sock = queue->front();
        queue->pop();
    } catch(const std::exception &e) {
        print("Error: Popping from empty queue");
        sock = nullptr;
    }
    pthread_mutex_unlock(&this->popLock);
    return sock;
}

void TWT_Peer::TWT_AwaitSocket(TWT_Thread *caller) {
    while(true) {
//        print("Thread ",caller->id,": Awaiting connection");
        pthread_mutex_lock(&this->socketLock);
        while (this->receive_sockets.size() == 0) {
            pthread_cond_wait(&this->socketSignal,&this->socketLock);
        }

//        print("Dispatching thread ",thread->id);
        tcp::socket *sock = this->TWT_SafePop(&this->receive_sockets);
        pthread_mutex_unlock(&this->socketLock);

        this->TWT_ServeSocket(sock,caller);
    }
}
void TWT_Peer::TWT_ServeSocket(tcp::socket *sock,TWT_Thread *caller) {
    print("Received connection from ",sock->remote_endpoint().address().to_string());
    char msg[1024];
    clear_buffer(msg,1024);
    asio::error_code error;
    size_t len = sock->read_some(asio::buffer(msg,1024), error);
    while(!error) {
        std::string s(msg);
        print(s);
        clear_buffer(msg,1024);
        size_t len = sock->read_some(asio::buffer(msg,1024), error);
    }
    print("Error:",error.message());
    print("Closed socket");
}

void TWT_Peer::TWT_Listen(TWT_Thread *caller) {

    print("Listening on ",this->port,"...");

    tcp::socket *sock;

    while(true) {
        try {
            sock = new tcp::socket(io_context);
            this->acceptor->accept(*sock);
        } catch (const std::exception &e) {
            print(e.what());
            return;
        }

//        pthread_mutex_lock(&this->socketLock);
        this->receive_sockets.push(sock);
        pthread_cond_signal(&this->socketSignal);
//        pthread_mutex_unlock(&this->socketLock);

    }

}

void TWT_Peer::TWT_Send(const std::string &s,tcp::socket *sock) {
    asio::error_code error;
    size_t len = asio::write(*sock, asio::buffer(s), error);
    if (error) print("Error: ", error.message());
    print("Wrote ", len, " bytes to socket");
}

bool TWT_Peer::TWT_Connect(const std::string &host) {
    print("Connecting to ",host,":",this->port);
    tcp::socket *sock;

    try {
        sock = new tcp::socket(io_context);
        tcp::resolver::results_type endpoints = this->resolver->resolve(host, std::to_string(this->port));
        asio::connect(*sock, endpoints);
    } catch(const std::exception &e) {
        print(e.what());
        return false;
    }
    print("Connected to ",host);
    this->send_sockets.push(sock);
    this->TWT_Send("Test message",sock);
//    this->TWT_Send("Test message");
//    this->TWT_CloseSocket(sock);
    return true;

}