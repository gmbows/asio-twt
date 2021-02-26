#pragma once
#include <tuple>
#include <iostream>

class TWT_Peer;
struct TWT_Thread;

struct TWT_ThreadPackage {
    std::tuple<TWT_Peer*,TWT_Thread*> package;

    TWT_ThreadPackage(TWT_Peer *peer,TWT_Thread *thread) {
        this->package = std::make_tuple(peer,thread);
    }
};