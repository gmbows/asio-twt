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
        std::queue<tcp::socket*> outgoing_sockets;
        std::queue<tcp::socket*> incoming_sockets;

        pthread_mutex_t incomingSocketLock = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t outgoingSocketLock = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t popLock = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t receivedConnection = PTHREAD_COND_INITIALIZER;
        pthread_cond_t madeConnection = PTHREAD_COND_INITIALIZER;

        TWT_ListenerThread *listener = new TWT_ListenerThread();

        TWT_ThreadPool<TWT_IncomingSocketThread> incomingThreadPool;
        TWT_ThreadPool<TWT_OutgoingSocketThread> outgoingThreadPool;

        int port;

        bool active;
        bool listening;

        //Server functionality
        void TWT_Listen(TWT_Thread*);
        void TWT_Listen() {
//            print("Starting listener thread");
            this->listener->start(this);
        }

        //Thread routine for handling incoming connections
        void TWT_AwaitSocket(TWT_Thread*);
        void TWT_ServeSocket(tcp::socket*,TWT_Thread*);

        //Client functionality
        void TWT_Send(const std::string &msg,tcp::socket*);
        void TWT_GreetSocket(tcp::socket*,TWT_Thread*);
        bool TWT_Connect(const std::string &host);

        //Thread routine for handling outgoing connections
        void TWT_AwaitOutgoingSocket(TWT_Thread*);


        //Utility
        /*TODO:
         * Pass TWT_SafePop a queue and the correct lock
         * Currently we use the same lock for all queues
        */
        tcp::socket* TWT_SafePop(std::queue<tcp::socket*>*);

        void TWT_CloseSocket(tcp::socket *sock) {
            asio::error_code err;
            if(sock->is_open()) {
                sock->shutdown(tcp::socket::shutdown_send,err);
            }
            if(err) print("TWT_CloseSocket():",err.message());
        }

        TWT_Peer(int _port,int numThreads): port(_port),acceptor(new tcp::acceptor(io_context, tcp::endpoint(tcp::v4(), _port))),resolver(new tcp::resolver(io_context)) {
            pthread_cond_init(&this->receivedConnection,NULL);
            pthread_cond_init(&this->madeConnection,NULL);

            pthread_mutex_init(&this->incomingSocketLock,NULL);
            pthread_mutex_init(&this->outgoingSocketLock,NULL);
            pthread_mutex_init(&this->popLock,NULL);

            this->incomingThreadPool = TWT_ThreadPool<TWT_IncomingSocketThread>(10,this);
            this->outgoingThreadPool = TWT_ThreadPool<TWT_OutgoingSocketThread>(10,this);

            this->incomingThreadPool.start_threads();
            this->outgoingThreadPool.start_threads();

            this->active = true;
            this->listening = false;

            print("Peer initialized on port ",_port," with ",numThreads," connection threads");
        }
};