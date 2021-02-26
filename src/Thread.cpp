#include "Thread.h"

#include <pthread.h>

#include "Network.h"
#include "Common.h"
#include "NetworkUtils.h"

void TWT_ListenerThread::Listen(TWT_Peer *peer,TWT_ListenerThread *thread) {
    peer->TWT_Listen(thread);
}

void TWT_SocketThread::AwaitSocket(TWT_Peer *peer,TWT_SocketThread *thread) {
    peer->TWT_AwaitSocket(thread);
}