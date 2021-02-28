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
        std::queue<tcp::socket*> connections;
        std::queue<TWT_Packet*> pendingData;

        pthread_mutex_t readLock = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t writeLock = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t popLock = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t gotReadJob = PTHREAD_COND_INITIALIZER;
        pthread_cond_t gotWriteJob = PTHREAD_COND_INITIALIZER;

        TWT_ListenerThread *listener = new TWT_ListenerThread();

        TWT_ThreadPool<TWT_ReadThread> readers;
        TWT_ThreadPool<TWT_WriteThread> writers;

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
        void TWT_AwaitReadJob(TWT_Thread*);
        void TWT_ServeSocket(tcp::socket*,TWT_Thread*);

        //Client functionality
        void TWT_Send(TWT_Packet*);
        void TWT_GreetSocket(tcp::socket*,TWT_Thread*);
        bool TWT_Connect(const std::string &host);

        //Thread routine for handling outgoing connections
        void TWT_AwaitWriteJob(TWT_Thread*);

        //Utility
        /*TODO:
         * Pass TWT_SafePop a queue and the correct lock
         * Currently we use the same lock for all queues
        */
        tcp::socket* TWT_SafePop(std::queue<tcp::socket*>*);
        TWT_Packet* TWT_PopWriteQueue();

        void TWT_CloseSocket(tcp::socket *sock) {
            asio::error_code err;
            if(sock->is_open()) {
                sock->shutdown(tcp::socket::shutdown_send,err);
            }
            if(err) print("TWT_CloseSocket():",err.message());
        }

        TWT_Peer(int _port,int numThreads): port(_port),acceptor(new tcp::acceptor(io_context, tcp::endpoint(tcp::v4(), _port))),resolver(new tcp::resolver(io_context)) {

            this->listening = false;

            pthread_cond_init(&this->gotReadJob,NULL);
            pthread_cond_init(&this->gotWriteJob,NULL);

            pthread_mutex_init(&this->readLock,NULL);
            pthread_mutex_init(&this->writeLock,NULL);
            pthread_mutex_init(&this->popLock,NULL);

            this->readers = TWT_ThreadPool<TWT_ReadThread>(10,this);
            this->writers = TWT_ThreadPool<TWT_WriteThread>(10,this);

            this->readers.start();
            this->writers.start();

            this->active = true;

            print("Peer initialized on port ",_port," with ",numThreads," connection threads");
        }
};