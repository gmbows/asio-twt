#include "Thread.h"
void* run_thread(void *threadPtr) {
    static_cast<TWT_Thread*>(threadPtr)->active = true;
    static_cast<TWT_Thread*>(threadPtr)->routine(nullptr);
    static_cast<TWT_Thread*>(threadPtr)->active = false;
    return nullptr;
}