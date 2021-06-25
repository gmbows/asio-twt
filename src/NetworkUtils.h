#pragma once
#include <tuple>
#include <vector>
#include <iostream>
#include <asio.hpp>
#include <fstream>
#include <stack>

#include "Common.h"
#include "Utility.h"

class TWT_Peer;

using asio::ip::tcp;

std::string get_address(tcp::socket *sock);

struct TWT_File {
    std::string filename;
    std::vector<char> data;

    bool valid;

    bool import() {
		//If this is an existing file, import the file
        std::ifstream file(this->filename,std::ios::binary);
        if(file.is_open()) {
            char c;
            while(file >> std::noskipws >> c) {
                this->data.push_back(c);
            }
        }
        if(!file.is_open()) print("Error opening ",this->filename);
        return file.is_open();
    };
	
	bool write(std::vector<char> bytes) {
		std::ofstream file(this->filename,std::ios::binary);
        if(file.is_open()) {
			for(auto c : bytes) {
				file << c;
			}
			file.close();
        }
        // if(!file.is_open()) print("Error writing to ",this->filename);
        return true;
	}

    size_t inline size() {return this->data.size();}

    void close() {
        std::vector<char>().swap(this->data);
    }
	
	std::vector<char> serialized() {
		std::vector<char> serial;
		
		std::string fname = this->filename;
		pad(fname,TWT_PAD_FILENAME,"/");
		
		for(auto c : fname) {
			serial.push_back(c);
		}
		for(auto c : this->data) {
			serial.push_back(c);
		}
		return serial;
	}

    TWT_File(std::string _filename): filename(_filename) {
        this->valid = this->import();
    }

};

struct TWT_Transfer {
	unsigned int bytes_remaining;
	DataType type;
	std::string filename;
	std::vector<char> buffer;
	
	TWT_Transfer(){
		this->bytes_remaining = 1;
	}
};

struct TWT_TransferStack {
	std::stack<TWT_Transfer*> transfers;
	
	void create(unsigned int bytes,DataType type,const std::string &filename = "") {
		TWT_Transfer *transfer = new TWT_Transfer();
		transfer->bytes_remaining = bytes;
		transfer->filename = filename;
		transfer->type = type;
		this->transfers.push(transfer);
	}
	TWT_Transfer *top() {
		return this->transfers.top();
	}
	void pop() {
		this->transfers.pop();
	}
	TWT_TransferStack(){}
};

struct TWT_Connection {
	tcp::socket *sock;
	std::string sock_id;
	std::string address;
	unsigned int num_packets = 0;
	
	//File reading
	bool reading;
	DataType readingType;
	int bytesRemaining;
	std::string fname;
	TWT_TransferStack transfers;
	
	inline DataType reading_type() {
		return this->transfers.top()->type;
	}
	inline unsigned int bytes_remaining() {
		return this->transfers.top()->bytes_remaining;
	}
	void add_transfer(unsigned int bytes,DataType type,const std::string &filename = "") {
		this->transfers.create(bytes,type,filename);
	}
	TWT_Transfer* current_transfer() {
		return this->transfers.top();
	}
	std::vector<char>* current_transfer_buffer() {
		return &this->transfers.top()->buffer;
	}
	
	TWT_Connection(tcp::socket *_sock,std::string id, std::string addr): sock(_sock), sock_id(id), address(addr) {
		this->reading = false;
	}
	
	void reset() {
	// std::vector<char>().swap(this->buffer);
	this->transfers.pop();
	
	this->fname = "";
	// this->bytesRemaining = -1;
}

	
};

struct TWT_Packet {
    tcp::socket *sock;
	
	//Requred fields
	DataType type;
	unsigned int id;
	size_t size;
    std::vector<char> data;
	
	//Optional fields
	std::string filename;
	
	
	//Structure:
    // 2 digits for packet type
	// 20 digits for packet id
    // 16 digits for data size
	// 255 digits for filename
    // n digits  for data

    //Position of header index in data
    unsigned int pos = 0;
	
    void format_data();
	
	void emplace_packet_header() {
		for(int i=0;i<packet_header.size();i++) {
			this->data.emplace(data.begin()+i,packet_header[i]);
		}
	}

    void import_string(const std::string &message) {
        for(auto character : message) {
            this->data.push_back(character);
        }
    }
	
	bool validate_header(std::vector<char> vdata) {
		
		for(int i=0;i<packet_header.size();i++) {
			if(vdata.at(i) != packet_header[i]) {
				return false;
			}
		}
		return true;
	}

    void append_header(std::string s,int padding);
	
	void deserialize(std::vector<char> vdata);
	
    TWT_Packet(tcp::socket *_sock,const std::string &message): sock(_sock) {

        this->type = TWT_TEXT;
	
        this->import_string(message);

        this->format_data();
		this->emplace_packet_header();
        this->size = this->data.size();
    }

    TWT_Packet(tcp::socket *_sock,std::vector<char> _data, DataType _type): sock(_sock), data(_data), type(_type) {
		
        this->format_data();
		this->emplace_packet_header();
        this->size = this->data.size();
    }
	
	TWT_Packet(tcp::socket *_sock,const std::string &message, DataType _type): sock(_sock), type(_type) {
        this->import_string(message);

        this->format_data();
		this->emplace_packet_header();
        this->size = this->data.size();
    }

	TWT_Packet(std::vector<char> vdata) {
		if(this->validate_header(vdata)) {
			// print("Header validated");
			//If this packet has a valid header, import the data
			this->deserialize(vdata);
		} else {
			//If this packet doesn't have a valid header, assume it is raw data
			this->type = TWT_DATA;
			this->data = vdata;
		}
	}
	
	TWT_Packet(): type(TWT_DATA) {}
};

template <typename ThreadType>
struct TWT_ThreadPackage {
    std::tuple<TWT_Peer*,ThreadType*> package;

    TWT_ThreadPackage(TWT_Peer *peer,ThreadType *thread) {
        this->package = std::make_tuple(peer,thread);
    }
};