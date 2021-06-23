#pragma once
#include <tuple>
#include <vector>
#include <iostream>
#include <asio.hpp>
#include <fstream>

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

struct TWT_Packet {
    tcp::socket *sock;
    std::vector<char> data;
    DataType type;
    size_t size;

    //Position of header index in data
    unsigned int pos = 0;

    //Structure:
    // 1 byte for data type
    // 64 bytes for size
    // n bytes for data

    void format_data();

    void import_string(const std::string &message) {
        for(auto character : message) {
            this->data.push_back(character);
        }
    }

    void append_header(std::string s,int padding);

    TWT_Packet(tcp::socket *_sock,const std::string &message): sock(_sock) {

        this->type = DATA_MSG;
        this->import_string(message);

        this->format_data();
        this->size = this->data.size();
    }

    TWT_Packet(tcp::socket *_sock,std::vector<char> _data, DataType _type): sock(_sock), data(_data), type(_type) {
        this->format_data();
        this->size = this->data.size();
    }
	
	TWT_Packet(tcp::socket *_sock,const std::string &message, DataType _type): sock(_sock), type(_type) {
        this->import_string(message);

        this->format_data();
        this->size = this->data.size();
    }
};

template <typename ThreadType>
struct TWT_ThreadPackage {
    std::tuple<TWT_Peer*,ThreadType*> package;

    TWT_ThreadPackage(TWT_Peer *peer,ThreadType *thread) {
        this->package = std::make_tuple(peer,thread);
    }
};