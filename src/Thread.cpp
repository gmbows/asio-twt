#include "Thread.h"

#include <pthread.h>

#include "Utility.h"
#include "Network.h"
#include "Common.h"
#include "NetworkUtils.h"

void DefaultRoutine(TWT_Peer *peer,TWT_Thread *thread) {
    print("Error: Calling default thread routine (overwrite this)");
}

void Listen(TWT_Peer *peer,TWT_Thread *thread) {
    peer->TWT_Listen(thread);
}

void AwaitReadJob(TWT_Peer *peer,TWT_Thread *thread) {
    peer->TWT_AwaitReadJob(thread);
}

void AwaitWriteJob(TWT_Peer *peer,TWT_Thread *thread) {
    peer->TWT_AwaitWriteJob(thread);
}