#include "Network.h"
#include <asio.hpp>
#include <string>
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

TWT_Connection* TWT_Peer::TWT_PopReadQueue() {
    pthread_mutex_lock(&this->popLock);
    TWT_Connection *connection;
    try {
        connection = this->connections.front();
        this->connections.pop();
    } catch(const std::exception &e) {
        print("Error: Popping from empty queue");
        connection = nullptr;
    }
    pthread_mutex_unlock(&this->popLock);
    return connection;
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

            TWT_Connection *connection = this->TWT_PopReadQueue();
            pthread_mutex_unlock(&this->readLock);

            this->TWT_ServeConnection(connection, caller);
        } else {
            pthread_mutex_unlock(&this->readLock);
        }
    }
}

DataType TWT_Peer::HandlePacket(TWT_Connection *connection, TWT_Packet packet) {
    if(packet.data.size() == 0) return TWT_EMPTY;

    //If we are not currently reading some data
    // We should expect this packet to contain some header information
	// add this information to the transfer queue
    if(packet.type != TWT_DATA) {
		
		connection->reading = true;
		
		//Expect n bytes of data related to this packet
		if(packet.type == TWT_FILE) {
			connection->add_transfer(packet.size,packet.type,packet.filename);
		} else {
			connection->add_transfer(packet.size,packet.type);
		}
		
		//The rest of this packet is data, add it to the current transfer buffer
		packet.type = TWT_DATA;
		
    }
	
    if(connection->reading) {
        //We are reading data
		//If we reach the bytecount, decide what to do based on the transfer type
		// print(connection->bytes_remaining());
        for(auto c : packet.data) {
			if(packet.type == TWT_DATA) {
				connection->current_transfer_buffer()->push_back(c);
				connection->current_transfer()->bytes_remaining--;
			} else {
				print("ERROR: Unexpected packet type ",packet.type);
			}
            if(connection->current_transfer()->bytes_remaining == 0) {
                connection->reading = false;
				switch(connection->reading_type()) {	
					case TWT_FILE: {
						// Accept or reject file here
						std::string fname = connection->current_transfer()->filename;
						TWT_File f = TWT_File(fname);
						f.write(*connection->current_transfer_buffer());
						print("Received remote file \"",fname,"\"");
						this->TWT_PackageAndSend(fname, connection->sock,TWT_FILE_ACK);
						connection->reset();
						break;
					}
					case TWT_TEXT: {
						std::string msg;
						clean_vector(packet.data);
						for(auto c : packet.data) msg+=c;
						if(msg.size() > 0) print("(Remote) ",msg);
						connection->reset();
						break;
					}
					case TWT_FILE_ACK: {
						std::string filename;
						clean_vector(packet.data);
						for(auto c : packet.data) filename+=c;
						if(filename.size() > 0) print("(Remote) Received file ",filename);
						break;
					}
					case TWT_CLOSE: {
                        print("(Remote) Received disconnect");
						this->TWT_PackageAndSend("disc", connection->sock,TWT_ACK);
                        break;	
					}
					case TWT_SIGN: {
						print("(Remote) Received signal");
                        break;	
					}
					case TWT_ACK: {
						std::string id;
						clean_vector(packet.data);
						int i=0;
						for(auto c : *connection->current_transfer_buffer()) {
							id+=c;
							if(i++ == 6) break;
						}
						if(id.size() > 0) print("Remote host acknowledges packet ",id," from socket ",connection->sock_id);
						break;
					}
				}
				if(this->isAwaiting(connection->readingType)) {
					this->awaiting = false;
					pthread_cond_signal(&this->gotAwaitSignal);
				}
				break;
            }
        }
		// print(connection->bytes_remaining());
    }
	return packet.type;
}

void TWT_Peer::TWT_ServeConnection(TWT_Connection *connection,TWT_Thread *caller) {
    print("Received link from ",connection->address);
	
	//Buffer for reading
    char data[TWT_BUFFER_SIZE];
    clear_buffer(data,TWT_BUFFER_SIZE);
	
    std::vector<char> vdata;
	
    asio::error_code error;
	
    while(!error) {
        size_t len = connection->sock->read_some(asio::buffer(data,TWT_BUFFER_SIZE), error);

        vdata = array_to_vector(data,TWT_BUFFER_SIZE);
		
		TWT_Packet packet = TWT_Packet(vdata);
		
        this->HandlePacket(connection,packet);
		if(packet.type != TWT_ACK) {
			connection->num_packets++;
			// this->TWT_PackageAndSend(std::to_string(connection->num_packets), connection->sock,TWT_ACK);
		}

        clear_buffer(data,TWT_BUFFER_SIZE);
    }
    switch(error.value()) {
        case 2:
            break;
        default:
            print("Unexpected socket error: ",error.value(),", ",error.message());
    }

    this->TWT_MarkSocketForClosing(connection->sock);
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
	std::string sock_id = std::to_string(this->numConnections++);
	std::string address = get_address(sock);
	
    clean_insert(this->addressMap,sock_id,sock);

	TWT_Connection *new_connection = new TWT_Connection(sock,sock_id,address);
    this->connections.push(new_connection);
    pthread_cond_signal(&this->gotReadJob);

    print("Linked to ",address);
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

void TWT_Peer::TWT_PackageAndSend(const std::string &message,const std::string &socket_id,DataType type) {
    if(!contains(this->addressMap,socket_id)) {
        print("No active connection with id ",socket_id);
        return;
    }

    //Package message
    tcp::socket *sock = this->addressMap.at(socket_id);
    TWT_Packet *packet = new TWT_Packet(sock,message,type);
    this->pendingData.push(packet);

    //Send signal to deliver new write job
    pthread_cond_signal(&this->gotWriteJob);
}

void TWT_Peer::TWT_PackageAndSendForResponse(const std::string &message,const std::string &socket_id,DataType type,DataType returnType) {
    if(!contains(this->addressMap,socket_id)) {
        print("No active connection with id ",socket_id);
        return;
    }

    //Package message
    tcp::socket *sock = this->addressMap.at(socket_id);
    TWT_Packet *packet = new TWT_Packet(sock,message,type);
    this->pendingData.push(packet);

    //Send signal to deliver new write job
    pthread_cond_signal(&this->gotWriteJob);
	
	this->TWT_Await(returnType);
	
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

void TWT_Peer::TWT_AwaitTransfer(TWT_Thread *caller) {

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
            //if(err) print("TWT_CloseSocket():",err.message());
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

void TWT_Peer::TWT_HandleInput() {
	while(true) {
		char in[3];
		fgets(in,3,stdin);
		std::string s(in);
        if(s.size() > 0) print(s);
		//SDL_Delay(1000/60);
	}
}
