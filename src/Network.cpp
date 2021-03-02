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
        pthread_mutex_lock(&this->readLock);
        while (this->connections.size() == 0) {
            pthread_cond_wait(&this->gotReadJob,&this->readLock);
        }
        tcp::socket *sock = this->TWT_SafePop(&this->connections);
        pthread_mutex_unlock(&this->readLock);

        this->TWT_ServeSocket(sock,caller);
    }
}
void TWT_Peer::TWT_ServeSocket(tcp::socket *sock,TWT_Thread *caller) {
    print("Received connection from ",get_address(sock));
    char msg[TWT_BUFFER_SIZE];
    clear_buffer(msg,TWT_BUFFER_SIZE);
    asio::error_code error;
    while(!error) {
        size_t len = sock->read_some(asio::buffer(msg,TWT_BUFFER_SIZE), error);
        std::string s(msg);
        if(s.size() > 0) print(s);
        clear_buffer(msg,TWT_BUFFER_SIZE);
    }
    print("Error: ",error.message());
    this->TWT_CloseSocket(sock);
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
        clean_insert(this->addressMap,std::to_string(this->numConnections++),sock);
        this->connections.push(sock);
        TWT_Packet *packet = new TWT_Packet(sock,"SERV Welcome");
        this->pendingData.push(packet);
        pthread_cond_signal(&this->gotWriteJob);
        pthread_cond_signal(&this->gotReadJob);
//        pthread_mutex_unlock(&this->readLock);

    }

}

void TWT_Peer::TWT_SendPacket(TWT_Packet *packet) {
    asio::error_code error;
    size_t written = asio::write(*packet->sock, asio::buffer(packet->data,packet->size), error);
    if (error) print("Error (TWT_Send()): ", error.message());
//    print("Wrote ", written, " bytes to socket");
}

void TWT_Peer::TWT_FormatAndSend(const std::string &message,const std::string &socket_id) {
    if(!contains(this->addressMap,socket_id)) {
        print("No active connection with id ",socket_id);
        return;
    }

    //Package message
    tcp::socket *sock = this->addressMap.at(socket_id);
    TWT_Packet *packet = new TWT_Packet(sock,message);
    this->pendingData.push(packet);

    //Send signal to deliver new write job
    pthread_cond_signal(&this->gotWriteJob);
}

void TWT_Peer::TWT_AwaitWriteJob(TWT_Thread *caller) {
    while(this->active) {
        pthread_mutex_lock(&this->writeLock);
        while (this->pendingData.size() == 0) {
            pthread_cond_wait(&this->gotWriteJob,&this->writeLock);
        }

        TWT_Packet *packet = this->TWT_PopWriteQueue();
        pthread_mutex_unlock(&this->writeLock);

        this->TWT_SendPacket(packet);

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
    clean_insert(this->addressMap,std::to_string(this->numConnections++),sock);
    print("Connected to ",host);
    this->connections.push(sock);
    pthread_cond_signal(&this->gotReadJob);
//    this->TWT_CloseSocket(sock);
    return true;

}