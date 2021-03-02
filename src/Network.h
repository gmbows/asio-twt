#pragma once

#define _WIN32_WINDOWS

#include <asio.hpp>
#include <pthread.h>
#include <queue>
#include <map>

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
        pthread_mutex_t readLock = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t writeLock = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t popLock = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t gotReadJob = PTHREAD_COND_INITIALIZER;
        pthread_cond_t gotWriteJob = PTHREAD_COND_INITIALIZER;

        //A single listening thread
        TWT_ListenerThread *listener = new TWT_ListenerThread();

        //Queues that read/write threads work from
        std::queue<tcp::socket*> connections;
        std::queue<TWT_Packet*> pendingData;

        std::map<std::string,tcp::socket*> addressMap;

        TWT_ThreadPool<TWT_ReadThread> readers;
        TWT_ThreadPool<TWT_WriteThread> writers;

        int port;

        bool active;
        bool listening;

        int numConnections;

        //Server functionality
        void TWT_Listen(TWT_Thread*);
        void TWT_Listen() {
            if(this->listening) {
                print("Already listening");
                return;
            }
            this->listener->start(this);
        }

        //Thread routine for handling incoming connections
        void TWT_AwaitReadJob(TWT_Thread*);
        void TWT_ServeSocket(tcp::socket*,TWT_Thread*);

        //Client functionality
        void TWT_SendPacket(TWT_Packet*);
        void TWT_FormatAndSend(const std::string&,const std::string&);
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
            for(auto it = this->addressMap.begin();it != this->addressMap.end();it++) {
                if((char*)it->second == (char*)sock) {
                    print("Closed connection ",it->first);
                    this->addressMap.erase(it);
                    break;
                }
            }
            if(err) print("TWT_CloseSocket():",err.message());
        }

    void TWT_CloseConnection(const std::string &sock_id) {
        asio::error_code err;
        if(contains(this->addressMap,sock_id)) {
            this->TWT_CloseSocket(this->addressMap.at(sock_id));
        }
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

            this->numConnections = 0;

            this->active = true;

            print("Peer initialized on port ",_port," with ",numThreads," connection threads");
        }
};