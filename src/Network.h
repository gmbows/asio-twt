#pragma once

#define _WIN32_WINDOWS

#include <asio.hpp>
#include <pthread.h>
#include <queue>

#include "Common.h"
#include "Utility.h"
#include "Thread.h"
#include "NetworkUtils.h"

using asio::ip::tcp;

class TWT_Peer {
    private:
        tcp::resolver *resolver;
        tcp::acceptor *acceptor;
    public:
        std::queue<tcp::socket*> send_sockets;
        std::queue<tcp::socket*> receive_sockets;

        pthread_mutex_t socketLock = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t popLock = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t socketSignal = PTHREAD_COND_INITIALIZER;

        TWT_ListenerThread *listener = new TWT_ListenerThread();
        TWT_ThreadPool<TWT_SocketThread> threadPool;

        int port;

        bool active = true;

        //Server functionality
        void TWT_Listen(TWT_Thread*);
        void TWT_Listen() {
            print("Starting listener thread");
            this->listener->start(this);
        }

        void TWT_AwaitSocket(TWT_Thread*);
        void TWT_ServeSocket(tcp::socket*,TWT_Thread*);

        //Client functionality
        void TWT_Send(const std::string &msg,tcp::socket*);
        bool TWT_Connect(const std::string &host);

        //Utility
        tcp::socket* TWT_SafePop(std::queue<tcp::socket*>*);

        void TWT_CloseSocket(tcp::socket *sock) {
            asio::error_code err;
            if(sock->is_open()) {
                sock->shutdown(tcp::socket::shutdown_send,err);
            }
            if(err) print("TWT_CloseSocket():",err.message());
        }

        TWT_Peer(int _port,int numThreads): port(_port),acceptor(new tcp::acceptor(io_context, tcp::endpoint(tcp::v4(), _port))),resolver(new tcp::resolver(io_context)) {
            pthread_cond_init(&this->socketSignal,NULL);
            pthread_mutex_init(&this->socketLock,NULL);
            pthread_mutex_init(&this->popLock,NULL);

            this->threadPool = TWT_ThreadPool<TWT_SocketThread>(10,this);
            this->threadPool.start_threads();
            print("Peer initialized on port ",_port," with ",numThreads," connection threads");
        }
};