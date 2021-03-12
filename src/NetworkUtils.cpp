#include "NetworkUtils.h"

#include "Utility.h"
std::string get_address(tcp::socket *sock) {
    return sock->remote_endpoint().address().to_string();
}

void TWT_Packet::format_data() {
    //Affix data type

    std::string type = std::to_string((int)this->type);
    std::string size = std::to_string(this->data.size());

    pad(type,TWT_PAD_TYPE,"0");
    pad(size,TWT_PAD_SIZE,"0");

    int i = 0;

    for(char c : type) {
        this->data.emplace(this->data.begin()+i, c);
        i++;
    }
    for(char c : size) {
        this->data.emplace(this->data.begin()+i, c);
        i++;
    }
}