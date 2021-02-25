#pragma once

#define _WIN32_WINDOWS

#include <asio.hpp>

#include "Common.h"
#include "Utility.h"

using asio::ip::tcp;

class TWT_Peer {
    private:
        tcp::resolver *resolver;
        tcp::acceptor *acceptor;
        tcp::socket *socket = new tcp::socket(io_context);
    public:
        int port;

        bool active = true;
        bool reading = false;
        int bytesLeft = 0;

        //Server functionality
        void TWT_HandleConnection();
        void TWT_Listen();

        //Client functionality
        void TWT_Send(const std::string &fn);
        bool TWT_Connect(const std::string &host);

        TWT_Peer(int _port): port(_port),acceptor(new tcp::acceptor(io_context, tcp::endpoint(tcp::v4(), _port))),resolver(new tcp::resolver(io_context)) {
            print("Peer initialized using port",_port);
        }
};