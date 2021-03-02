#include "Utility.h"
#include <string>
#include <fstream>
#include <sstream>

std::stringstream statement;

std::string operator *(const std::string &s, int len) {
    std::string output = "";
    for(int i=0;i<len;++i) {
        output += s;
    }
    return output;
}

//Converts char* array of length len to string vector
std::vector<std::string> convert_char_array(char** arr,int len) {
    std::vector<std::string> v;
    for(int i=0;i<len;++i) {
        std::string s(arr[i]);
        v.push_back(s);
    }
    return v;
}

void clear_buffer(char* buf,int len) {
    for(int i=0;i<len;++i) {
        buf[i] = '\0';
    }
}

std::vector<std::string> split(const std::string &s,char token) {
    std::vector<std::string> v;
    std::string run = "";
    char e;

    for(int i=0;i<s.size();++i) {
		e = s[i];
        if(e == token) {
            if(run.size() == 0) continue;
            v.push_back(run);
            run = "";
        } else {
            run += e;
        }
    }
    v.push_back(run);
    return v;
}

void get_and_tokenize_input(std::string &cmd, std::vector<std::string> &args) {
    std::string input;
    std::getline(std::cin,input);
    if(split(input,' ').size() == 0) return;
    cmd = split(input, ' ').at(0);
    args = split(input, ' ');
}

void tokenize(const std::string &input,std::string &cmd, std::vector<std::string> &args) {
    if(split(input,' ').size() == 0) return;
    cmd = split(input, ' ').at(0);
    args = split(input, ' ');
}

std::string command = "";
std::string cursor = ">>";

std::string read_command(const std::string &_cursor) {

    cursor = _cursor;

    std::string cmd = "";
    std::string space = " ";

    std::cout << "\r" << cursor << std::flush;

    while(true) {
        int keycode = getch();
        switch(keycode) {
            case 13:
                std::cout << std::endl;
                command = "";
                return cmd;
            case 3:
                return "quit";
            case 8:
                cmd = cmd.substr(0,cmd.size()-1);
                break;
            default:
                if(keycode >= 0 and keycode <= 255) {
                    cmd += (char)keycode;
                } else {
                }
                break;
        }

        //Update global command
        command = cmd;

        //Clear current terminal line
        // +1 for backspace
        std::cout << "\r" << space*(cursor.size()+cmd.size()+1) << "\r" << std::flush;
        std::cout << cursor << cmd << std::flush;
    }
    return "ERR_COMMAND_INVALID";
}

size_t import_file(const std::string &filename,char* &data) {
    std::ifstream image(filename,std::ios::binary);

    if(!image.is_open()) {
        print("(import) Error opening ",filename);
        return false;
    }

    //Seek to EOF and check position in stream
    image.seekg(0,image.end);
    size_t size = image.tellg();
    image.seekg(0,image.beg);

    data = (char*)malloc(size);

    char c;
    int i = 0;
    while(image >> std::noskipws >> c) {
         data[i++] = c;
    }

    image.close();
    return size;
}