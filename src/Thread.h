#pragma once

#include <pthread.h>
#include <vector>
#include <asio.hpp>
#include "Utility.h"
#include "NetworkUtils.h"
#include "Common.h"

using asio::ip::tcp;

class TWT_Peer;
class TWT_Thread;

typedef void (*ThreadRoutine)(TWT_Peer*,TWT_Thread*);

void DefaultRoutine(TWT_Peer*,TWT_Thread*);
void AwaitReadJob(TWT_Peer*,TWT_Thread*);
void AwaitWriteJob(TWT_Peer*,TWT_Thread*);
void AwaitTransfer(TWT_Peer*,TWT_Thread*);
void AwaitCloseJob(TWT_Peer*,TWT_Thread*);
void Listen(TWT_Peer*,TWT_Thread*);
void HandleInput(TWT_Peer*,TWT_Thread*);

/* TWT_Thread():
 *      TWT_Thread: generic pthread that performs services for peer objects.
 *
 *
 *      TWT_SocketThread: TWT_Thread that services sockets after they are received by a listener
 *      TWT_ListenerThread: TWT_Thread that listens for incoming connections for a peer object
 *
 *      Thread routines are defined in class constructor
 *
*/

struct TWT_Thread {
    int id;
    bool active;
    pthread_t thread;

    ThreadRoutine routine;

    void join()  {
        pthread_join(this->thread,NULL);
    }

    static void* run(void *ptr) {
        TWT_ThreadPackage<TWT_Thread> *package = static_cast<TWT_ThreadPackage<TWT_Thread>*>(ptr);
        TWT_Peer *peer = std::get<0>(package->package);
        TWT_Thread *thread = static_cast<TWT_Thread*>(std::get<1>(package->package));

        //Call specific thread type routine
        thread->routine(peer,thread);

        return nullptr;
    }

    bool start(TWT_Peer *peer) {
        if(pthread_create(&this->thread,NULL,&this->run,static_cast<void*>(new TWT_ThreadPackage<TWT_Thread>(peer,static_cast<TWT_Thread*>(this)))) != 0) {
            return false;
        }
        return true;
    }

    TWT_Thread() {
        this->active = false;
        this->id = ++thread_count;
        this->routine = &DefaultRoutine;
    }
};

//Listens for incoming connections
struct TWT_ListenerThread: public TWT_Thread {

    //Call TWT_Thread constructor
    TWT_ListenerThread(): TWT_Thread() {
        this->routine = &Listen;
    }
};

//Sends data to outgoing connections
struct TWT_InputThread: public TWT_Thread {

    //Call TWT_Thread constructor
    TWT_InputThread(): TWT_Thread() {
        this->routine = &HandleInput;
    }
};

//Receives data from incoming connections
struct TWT_ReadThread: public TWT_Thread {

    //Call TWT_Thread constructor
    TWT_ReadThread(): TWT_Thread() {
        this->routine = &AwaitReadJob;
    }
};

//Sends data to outgoing connections
struct TWT_WriteThread: public TWT_Thread {

    //Call TWT_Thread constructor
    TWT_WriteThread(): TWT_Thread() {
        this->routine = &AwaitWriteJob;
    }
};

//Transfers user data between peers
struct TWT_TransferThread: public TWT_Thread {

    //Call TWT_Thread constructor
    TWT_TransferThread(): TWT_Thread() {
        this->routine = &AwaitTransfer;
    }
};

//Closes pending closed sockets
struct TWT_CloserThread: public TWT_Thread {

    //Call TWT_Thread constructor
    TWT_CloserThread(): TWT_Thread() {
        this->routine = &AwaitCloseJob;
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

    bool start() {
        for(auto &thread : this->threads) {
            if(!thread->start(this->peer)) return false;
        }
        return true;
    }

    bool stop() {
        for(auto &thread : this->threads) {
            thread->join();
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