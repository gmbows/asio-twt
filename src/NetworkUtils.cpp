#include "NetworkUtils.h"

#include "Utility.h"
std::string get_address(tcp::socket *sock) {
    return sock->remote_endpoint().address().to_string();
}

void TWT_Packet::format_data() {
    //Affix version control header, data type, and packet size
    std::string type = std::to_string((int)this->type);
    std::string size = std::to_string(this->data.size());

    pad(type,TWT_PAD_TYPE,"0");
    pad(size,TWT_PAD_SIZE,"0");

    for(char c : type) {
        this->data.emplace(this->data.begin()+this->pos, c);
        this->pos++;
    }
    for(char c : size) {
        this->data.emplace(this->data.begin()+this->pos, c);
        this->pos++;
    }
}

void TWT_Packet::deserialize(std::vector<char> vdata) {
		//Extract data type from packet
		
		//Erase packet header
		for(int i=0;i<packet_header.size();i++) vdata.erase(vdata.begin());

		std::string type_padded;
        for (int i = 0; i < TWT_PAD_TYPE; i++) {
			
			try {
				type_padded += vdata.at(i);
			} catch(const std::exception &e) {
				print("Found padding error while reading data type");
				type_padded = std::to_string(i);
				break;
			}
        }
		vdata.erase(vdata.begin(), vdata.begin() + TWT_PAD_TYPE);
	   
		try {
			this->type = (DataType)std::stoi(type_padded);
		} catch(const std::exception &e) {
			print("Error converting data type to integer");
		}   

        // print("Reading packet with data type: ",this->type);

		std::string size_padded;
        for (int i = 0; i < TWT_PAD_SIZE; i++) {
			
			try {
				size_padded += vdata.at(i);
			} catch(const std::exception &e) {
				print("Found padding error while reading message length");
				size_padded = std::to_string(i);
				break;
			}
        }
		vdata.erase(vdata.begin(), vdata.begin() + TWT_PAD_SIZE);

        try {
            this->size = std::stoi(size_padded);
        } catch (const std::exception &e) {
            print("Error converting message length to integer (",size,")");
			return;
        }
		
		// print("Message payload size: ",this->bytesRemaining);

		//If we are reading a file
		//Extract the filename from the packet
		if(this->type == TWT_FILE) {
			//Delete filename padding from data length
			for (int i = 0; i < TWT_PAD_FILENAME; i++) {
				try {
					this->filename += vdata.at(i);
				} catch(const std::exception &e) {
					print("Found padding error while reading filename");
					break;
				}
			}

			this->size -= 255;
			vdata.erase(vdata.begin(), vdata.begin() + TWT_PAD_FILENAME);
			while(this->filename[0] == '/') this->filename.erase(this->filename.begin());
		}
		
		this->data = vdata;
		
	}
