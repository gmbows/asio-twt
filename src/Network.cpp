#include "Network.h"
#include <asio.hpp>
#include <string>
#include <queue>
#include <iostream>

#include "Utility.h"
#include "Common.h"
#include "NetworkUtils.h"
// #include "Command.h"
//#include <SDL2/SDL.h>

using asio::ip::tcp;

tcp::socket* TWT_Peer::TWT_PopClosingQueue() {
    pthread_mutex_lock(&this->popLock);
    tcp::socket *sock;
    try {
        sock = this->pendingClosings.front();
        this->pendingClosings.pop();
    } catch(const std::exception &e) {
        print("Error: Popping from empty queue");
        sock = nullptr;
    }
    pthread_mutex_unlock(&this->popLock);
    return sock;
}

tcp::socket* TWT_Peer::TWT_PopReadQueue() {
    pthread_mutex_lock(&this->popLock);
    tcp::socket *sock;
    try {
        sock = this->connections.front();
        this->connections.pop();
    } catch(const std::exception &e) {
        print("Error: Popping from empty queue");
        sock = nullptr;
    }
    pthread_mutex_unlock(&this->popLock);
    return sock;
}

TWT_Packet* TWT_Peer::TWT_PopWriteQueue() {
    pthread_mutex_lock(&this->popLock);
    TWT_Packet *packet;
    try {
        packet = this->pendingData.front();
        this->pendingData.pop();
    } catch(const std::exception &e) {
        print("Error: Popping from empty queue");
        packet = nullptr;
    }
    pthread_mutex_unlock(&this->popLock);
    return packet;
}

void TWT_Peer::TWT_AwaitReadJob(TWT_Thread *caller) {
    while(this->active) {
        pthread_mutex_lock(&this->readLock);
        while (this->active and this->connections.size() == 0) {
            pthread_cond_wait(&this->gotReadJob,&this->readLock);
        }

        if(this->active) {

            tcp::socket *sock = this->TWT_PopReadQueue();
            pthread_mutex_unlock(&this->readLock);

            this->TWT_ServeSocket(sock, caller);
        } else {
            pthread_mutex_unlock(&this->readLock);
        }
    }
}

void TWT_Peer::HandlePacket(tcp::socket *sock, std::vector<char> data) {
    if(data.size() == 0) return;

    //If we are not currently reading some data
    // We should expect this packet to contain some header information
    if(!this->reading) {

		//This is a placeholder
		//Extract data type from packet
        this->readingType = (DataType)(data.at(1)-'0');
        data.erase(data.begin(),data.begin()+TWT_PAD_TYPE);

        // print("Reading packet with data type: ",this->readingType);
		
		std::string size;
		bool padded = true;
        for (int i = 0; i < TWT_PAD_SIZE; i++) {
			
			try {
				size += data.at(i);
			} catch(const std::exception &e) {
				print("Found padding error while reading message length");
				size = std::to_string(i);
				padded = false;
				break;
			}
        }

		//if(padded) should be factored out eventually
        if(padded) data.erase(data.begin(), data.begin() + TWT_PAD_SIZE);

		for(auto c : data) std::cout << c << std::flush;
		std::cout << std::endl;

        try {
            this->bytesRemaining = std::stoi(size);
        } catch (const std::exception &e) {
            print("Error converting message length to integer");
            return;
        }
		
		//If we are reading a file
		//Extract the filename from the packet
		if(this->readingType == DATA_FILE) {
			//Delete filename padding from data length
			this->bytesRemaining -= 255;
			for (int i = 0; i < TWT_PAD_FILENAME; i++) {
				try {
					this->fname += data.at(i);
				} catch(const std::exception &e) {
					print("Found padding error while reading filename");
					break;
				}
			}
			data.erase(data.begin(), data.begin() + TWT_PAD_FILENAME);
			while(fname[0] == '/') this->fname.erase(this->fname.begin());
			// print("Receiving file "+this->fname);
		}
		
		if(this->bytesRemaining != (int)data.size()) {
			// print("Data length mismatch (",size," vs. ",data.size(),")");
		}
		
        this->reading = true;
    }
	
    if(this->reading) {
		
        //We are reading data
		//If we reach the bytecount, decide what to do based on the readingType
        for(auto c : data) {
			this->buffer.push_back(c);
            if(--this->bytesRemaining == 0) {
                this->reading = false;
				switch(this->readingType) {
					case DATA_FILE: {
						std::string msg;
						for(auto c : data) msg+=c;
						// Accept or reject file here
						TWT_File f = TWT_File(this->fname);
						f.write(this->buffer);
						print("Received remote file \"",this->fname,"\"");
						this->TWT_PackageAndSend(this->fname, sock,TWT_ACK);
						break;
					}
					case DATA_MSG: {
						std::string msg;
						clean_vector(data);
						for(auto c : data) msg+=c;
						if(msg.size() > 0) print("(Remote) ",msg);
						break;
					}
					case TWT_ACK: {
						std::string msg;
						clean_vector(data);
						for(auto c : data) msg+=c;
						if(msg.size() > 0) print("(Remote) Received file ",msg);
						break;
					}
				}
				this->reset();
            }
        }
    }
}

void TWT_Peer::TWT_ServeSocket(tcp::socket *sock,TWT_Thread *caller) {
    print("Received link from ",get_address(sock));
    char data[TWT_BUFFER_SIZE];
    clear_buffer(data,TWT_BUFFER_SIZE);
    std::vector<char> vdata;
    asio::error_code error;
    while(!error) {
        size_t len = sock->read_some(asio::buffer(data,TWT_BUFFER_SIZE), error);

        vdata = array_to_vector(data,TWT_BUFFER_SIZE);
        // vdata = clean_vector(vdata);
        this->HandlePacket(sock,vdata);

        clear_buffer(data,TWT_BUFFER_SIZE);
    }
    switch(error.value()) {
        case 2:
            break;
        default:
            print("Unexpected socket error: ",error.value(),", ",error.message());
    }
    this->TWT_MarkSocketForClosing(sock);
}

void TWT_Peer::TWT_Listen(TWT_Thread *caller) {

    print("Joined peer network on port ",this->port);
    this->listening = true;

    tcp::socket *sock;

    while(true) {
        try {
            sock = new tcp::socket(io_context);
            this->acceptor->accept(*sock);
        } catch (const std::exception &e) {
            print(e.what());
            return;
        }
        this->TWT_Link(sock);
    }
}

void TWT_Peer::TWT_Link(tcp::socket *sock) {
    clean_insert(this->addressMap,std::to_string(this->numConnections++),sock);

    this->connections.push(sock);
    pthread_cond_signal(&this->gotReadJob);

    print("Linked to ",get_address(sock));
    TWT_Packet *packet = new TWT_Packet(sock,"Link successful");
    this->pendingData.push(packet);
    pthread_cond_signal(&this->gotWriteJob);
}

void TWT_Peer::TWT_SendPacket(TWT_Packet *packet) {
    asio::error_code error;
    size_t written = asio::write(*packet->sock, asio::buffer(packet->data,packet->size), error);
    if (error) print("Error (TWT_Send()): ", error.message());
//    print("Wrote ", written, " bytes to socket");
}

void TWT_Peer::TWT_PackageAndSend(const std::string &message,const std::string &socket_id) {
    if(!contains(this->addressMap,socket_id)) {
        print("No active connection with id ",socket_id);
        return;
    }

    //Package message
    tcp::socket *sock = this->addressMap.at(socket_id);
    TWT_Packet *packet = new TWT_Packet(sock,message);
    this->pendingData.push(packet);

    //Send signal to deliver new write job
    pthread_cond_signal(&this->gotWriteJob);
}

void TWT_Peer::TWT_PackageAndSend(const std::string& message,tcp::socket *sock,DataType type) {

    //Package message
    TWT_Packet *packet = new TWT_Packet(sock,message,type);
    this->pendingData.push(packet);

    //Send signal to deliver new write job
    pthread_cond_signal(&this->gotWriteJob);
}

void TWT_Peer::TWT_PackageAndSend(std::vector<char> data,const std::string &socket_id,DataType type) {
    if(!contains(this->addressMap,socket_id)) {
        print("No active connection with id ",socket_id);
        return;
    }

    //Package message
    tcp::socket *sock = this->addressMap.at(socket_id);
    TWT_Packet *packet = new TWT_Packet(sock,data,type);
    this->pendingData.push(packet);

    //Send signal to deliver new write job
    pthread_cond_signal(&this->gotWriteJob);
}

void TWT_Peer::TWT_AwaitWriteJob(TWT_Thread *caller) {
    while(this->active) {
        pthread_mutex_lock(&this->writeLock);
        while (this->active and this->pendingData.size() == 0) {
            pthread_cond_wait(&this->gotWriteJob,&this->writeLock);
        }

        //If peer is no longer active when we wake up, return
        if(this->active) {

            TWT_Packet *packet = this->TWT_PopWriteQueue();
            pthread_mutex_unlock(&this->writeLock);

            this->TWT_SendPacket(packet);

        } else {
            pthread_mutex_unlock(&this->writeLock);
        }

    }
//    print("Write thread exiting");
}

bool TWT_Peer::TWT_Connect(const std::string &host) {

    tcp::socket *sock;

    try {
        sock = new tcp::socket(io_context);
        tcp::resolver::results_type endpoints = this->resolver->resolve(host, std::to_string(this->port));
        asio::connect(*sock, endpoints);
        this->TWT_Link(sock);
    } catch(const std::exception &e) {
        print(e.what());
        return false;
    }

    return true;

}

void TWT_Peer::TWT_CloseSocket(tcp::socket *sock) {
    asio::error_code err;
    try {
        if (sock->is_open()) {
            sock->shutdown(tcp::socket::shutdown_send, err);
            if(err) print("TWT_CloseSocket():",err.message());
        }
    } catch(const std::exception &e) {
        print("Error closing socket: ",e.what());
    }
    try {
        //Try to remove socket from connections list
        for(auto it = this->addressMap.begin();it != this->addressMap.end();it++) {
            if((char*)it->second == (char*)sock) {
                print("Closed connection ",it->first);
                this->addressMap.erase(it);
                break;
            }
        }
    } catch(const std::exception &e) {
        print("General error: ",e.what());
    }
	// this->numConnections--;
}

void TWT_Peer::TWT_MarkSocketForClosing(tcp::socket *sock) {
    this->pendingClosings.push(sock);
    pthread_cond_signal(&this->gotCloseJob);
}

void TWT_Peer::TWT_MarkSocketForClosing(const std::string &sock_id) {
    try {
        if(contains(this->addressMap,sock_id)) {
            this->TWT_MarkSocketForClosing(this->addressMap.at(sock_id));
        }
    } catch(const std::exception &e) {
        print("General error: ",e.what());
    }
}

void TWT_Peer::TWT_AwaitCloseJob(TWT_Thread *caller) {
    while(true) {
        pthread_mutex_lock(&this->closeLock);
        while (this->pendingClosings.size() == 0) {
            pthread_cond_wait(&this->gotCloseJob,&this->closeLock);
        }

        tcp::socket *sock = this->TWT_PopClosingQueue();
        pthread_mutex_unlock(&this->closeLock);

        this->TWT_CloseSocket(sock);
    }
}

void TWT_Peer::reset() {
	std::vector<char>().swap(this->buffer);
	this->fname = "";
	// this->bytesRemaining = -1;
}

void TWT_Peer::TWT_HandleInput() {
	while(true) {
		char in[3];
		fgets(in,3,stdin);
		std::string s(in);
        if(s.size() > 0) print(s);
		//SDL_Delay(1000/60);
	}
}
