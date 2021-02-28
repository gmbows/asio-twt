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

TWT_Packet* TWT_Peer::TWT_PopWriteQueue() {
    pthread_mutex_lock(&this->popLock);
    TWT_Packet *packet;
    try {
        packet = this->pendingData.front();
        this->pendingData.pop();
    } catch(const std::exception &e) {
        print("Error: Popping from empty queue");
        packet = nullptr;
    }
    pthread_mutex_unlock(&this->popLock);
    return packet;
}

void TWT_Peer::TWT_AwaitReadJob(TWT_Thread *caller) {
    while(true) {
//        print("Thread ",caller->id,": Awaiting connection");
        pthread_mutex_lock(&this->readLock);
        while (this->connections.size() == 0) {
            pthread_cond_wait(&this->gotReadJob,&this->readLock);
        }

//        print("Dispatching thread ",thread->id);
        tcp::socket *sock = this->TWT_SafePop(&this->connections);
        pthread_mutex_unlock(&this->readLock);

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
    this->listening = true;

    tcp::socket *sock;

    while(true) {
        try {
            sock = new tcp::socket(io_context);
            this->acceptor->accept(*sock);
        } catch (const std::exception &e) {
            print(e.what());
            return;
        }

//        pthread_mutex_lock(&this->readLock);
        this->connections.push(sock);
        TWT_Packet *packet = new TWT_Packet(sock,"Greetings from server");
        this->pendingData.push(packet);
        pthread_cond_signal(&this->gotWriteJob);
        pthread_cond_signal(&this->gotReadJob);
//        pthread_mutex_unlock(&this->readLock);

    }

}

void TWT_Peer::TWT_Send(TWT_Packet *packet) {
    asio::error_code error;
    size_t written = asio::write(*packet->sock, asio::buffer(packet->data,packet->size), error);
    if (error) print("Error: ", error.message());
    print("Wrote ", written, " bytes to socket");
}

void TWT_Peer::TWT_GreetSocket(tcp::socket *sock,TWT_Thread *caller) {
//    this->TWT_Send("Hello".c_str(),sock,1024);
}

void TWT_Peer::TWT_AwaitWriteJob(TWT_Thread *caller) {
    while(this->active) {
        pthread_mutex_lock(&this->writeLock);
        while (this->pendingData.size() == 0) {
            pthread_cond_wait(&this->gotWriteJob,&this->writeLock);
        }

        TWT_Packet *packet = this->TWT_PopWriteQueue();
        pthread_mutex_unlock(&this->writeLock);

        this->TWT_Send(packet);

        //Perform write
//        this->TWT_GreetSocket(sock,caller);

    }
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
    this->connections.push(sock);

    TWT_Packet *packet = new TWT_Packet(sock,"Greetings from client");
    this->pendingData.push(packet);
    pthread_cond_signal(&this->gotWriteJob);
    pthread_cond_signal(&this->gotReadJob);
//    this->TWT_CloseSocket(sock);
    return true;

}