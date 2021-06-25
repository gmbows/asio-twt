#include "Thread.h"

#include <pthread.h>

#include "Utility.h"
#include "Network.h"
#include "Common.h"
#include "NetworkUtils.h"

void handle_input();

void DefaultRoutine(TWT_Peer *peer,TWT_Thread *thread) {
    print("Error: Calling default thread routine (overwrite this)");
}

void Listen(TWT_Peer *peer,TWT_Thread *thread) {
    peer->TWT_Listen(thread);
}

void HandleInput(TWT_Peer *peer,TWT_Thread *thread) {
   peer->TWT_HandleInput();
}

void AwaitReadJob(TWT_Peer *peer,TWT_Thread *thread) {
    peer->TWT_AwaitReadJob(thread);
}

void AwaitWriteJob(TWT_Peer *peer,TWT_Thread *thread) {
    peer->TWT_AwaitWriteJob(thread);
}

void AwaitTransfer(TWT_Peer *peer,TWT_Thread *thread) {
    peer->TWT_AwaitTransfer(thread);
}

void AwaitCloseJob(TWT_Peer *peer,TWT_Thread *thread) {
    peer->TWT_AwaitCloseJob(thread);
}