#pragma once

#include <pthread.h>
#include <vector>
#include "Utility.h"

typedef void* (*ThreadRoutine)(void*);

struct TWT_Thread;
void* run_thread(void*);

struct TWT_Thread {
    int id;
    bool active;
    ThreadRoutine routine;

    bool start() {
        if(pthread_create(&this->thread,NULL,&run_thread,static_cast<void*>(this)) != 0) {
            return false;
        }
        return true;
    }

    pthread_t thread;

    TWT_Thread(ThreadRoutine _routine): routine(_routine) {
        this->active = false;
    }
    TWT_Thread() {}
};

struct TWT_ThreadPool {

    bool initialized;
    int numThreads;

    //pointer to function to operate on
    ThreadRoutine routine;

    bool init() {
        for (int i = 0; i < this->numThreads; ++i) {
            this->threads.push_back(new TWT_Thread(this->routine));
        }
        this->initialized = true;
        return true;
    }

    bool start_threads() {
        for(auto &thread : this->threads) {
            if(!thread->start()) return false;
        }
        return true;
    }

    std::vector<TWT_Thread*> threads;

    TWT_ThreadPool(int _numThreads,ThreadRoutine _routine): numThreads(_numThreads),routine(_routine) {
        this->initialized = false;
        this->init();
    }

};