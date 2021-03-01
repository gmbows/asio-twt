#include "NetworkUtils.h"

std::string get_address(tcp::socket *sock) {
    return sock->remote_endpoint().address().to_string();
}