#pragma once

#include <pthread.h>
#include <vector>
#include <asio.hpp>
#include "Utility.h"
#include "NetworkUtils.h"
#include "Common.h"

struct ThreadPackage;

using asio::ip::tcp;

typedef void* (*ThreadRoutine)(void*);

class TWT_Peer;

struct TWT_Thread {
    int id;
    bool active;
    pthread_t thread;

    static void* run(void*) {
        print("Error: Trying to run default TWT_Thread routine");
        return nullptr;
    }

    bool start(TWT_Peer *peer) {
        if(pthread_create(&this->thread,NULL,&this->run,static_cast<void*>(new TWT_ThreadPackage(peer,this))) != 0) {
            return false;
        }
        return true;
    }

    TWT_Thread() {
        this->active = false;
        this->id = ++thread_count;
    }
};

//Receives data from incoming sockets
struct TWT_ListenerThread: public TWT_Thread {

    //Passthrough to peer Listen method
    void Listen(TWT_Peer*,TWT_ListenerThread*);

    static void* run(void *ptr) {
        TWT_ThreadPackage *package = static_cast<TWT_ThreadPackage*>(ptr);
        TWT_Peer *peer = std::get<0>(package->package);
        TWT_ListenerThread *thread = static_cast<TWT_ListenerThread*>(std::get<1>(package->package));

        thread->Listen(peer,thread);

        return nullptr;
    }

    bool start(TWT_Peer *peer) {
        if(pthread_create(&this->thread,NULL,&this->run,static_cast<void*>(new TWT_ThreadPackage(peer,this))) != 0) {
            return false;
        }
        return true;
    }

    //Call TWT_Thread constructor
    TWT_ListenerThread(): TWT_Thread() {
    }
};

//Receives data from incoming sockets
struct TWT_SocketThread: public TWT_Thread {

    //Passthrough to peer AwaitSocket method
    static void AwaitSocket(TWT_Peer*,TWT_SocketThread*);

    static void* run(void *ptr) {
        TWT_ThreadPackage *package = static_cast<TWT_ThreadPackage*>(ptr);
        TWT_Peer *peer = std::get<0>(package->package);
        TWT_SocketThread *thread = static_cast<TWT_SocketThread*>(std::get<1>(package->package));

        thread->AwaitSocket(peer,thread);

        return nullptr;
    }

    bool start(TWT_Peer *peer) {
        if(pthread_create(&this->thread,NULL,&this->run,static_cast<void*>(new TWT_ThreadPackage(peer,this))) != 0) {
            return false;
        }
        return true;
    }

    //Call TWT_Thread constructor
    TWT_SocketThread(): TWT_Thread() {
    }
};

template <class ThreadType>
struct TWT_ThreadPool {

    bool initialized;
    int numThreads;

    TWT_Peer *peer;

    bool init() {
        for (int i = 0; i < this->numThreads; ++i) {
            this->threads.push_back(new ThreadType());
        }
        this->initialized = true;
        return true;
    }

    bool start_threads() {
        for(auto &thread : this->threads) {
            if(!thread->start(this->peer)) return false;
        }
        return true;
    }

    std::vector<ThreadType*> threads;

    TWT_ThreadPool(int _numThreads,TWT_Peer* _peer): numThreads(_numThreads),peer(_peer) {
        this->initialized = false;
        this->init();
    }

    TWT_ThreadPool(){}

};