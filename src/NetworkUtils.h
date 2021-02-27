#pragma once
#include <tuple>
#include <iostream>

class TWT_Peer;

template <typename ThreadType>
struct TWT_ThreadPackage {
    std::tuple<TWT_Peer*,ThreadType*> package;

    TWT_ThreadPackage(TWT_Peer *peer,ThreadType *thread) {
        this->package = std::make_tuple(peer,thread);
    }
};