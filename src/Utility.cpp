#include "Utility.h"
#include <string>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <conio.h>
#define KEYCODE_RETURN 13
#define KEYCODE_BS 8
#else
#include <termios.h>
#define KEYCODE_RETURN 10
#define KEYCODE_BS 127
static struct termios old,_new;

void initTermios() {
	tcgetattr(0,&old);
	_new = old;
	_new.c_lflag &= ~ICANON;
	_new.c_lflag &= ~ECHO;
	tcsetattr(0,TCSANOW,&_new);
}

void resetTermios() {
	tcsetattr(0,TCSANOW,&old);
}

int getch() {
	//std::cout << "Not supported yet!" << std::endl;
	char c;
	initTermios();
	c = getchar();
	resetTermios();
	return c;
}
#endif

std::stringstream statement;

std::string operator *(const std::string &s, int len) {
    std::string output = "";
    for(int i=0;i<len;++i) {
        output += s;
    }
    return output;
}

void clean_vector(std::vector<char> &v) {
    std::vector<char> n;
	while(v.at(v.size()-1) == '\0') {
		v.erase(v.begin()+(v.size()-1));
	}
	// return;
    // for(int i=0;i<v.size();++i) {
        // if(v.at(i) != '\0') {
            // n.push_back(v.at(i));
        // }
    // }
    // return n;
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

void get_and_tokenize_input(std::vector<std::string> &args) {
    std::string input;
    std::getline(std::cin,input);
    if(split(input,' ').size() == 0) return;
    args = split(input, ' ');
}

int tokenize(const std::string &input, std::vector<std::string> &args) {
//    if(split(input,' ').size() == 0) return 0;
    args = split(input, ' ');
    return args.size();
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
	//print("GOT KEYCODE",keycode);
        switch(keycode) {
            case KEYCODE_RETURN:
                std::cout << std::endl;
                command = "";
                return cmd;
            case 3:
                return "quit";
            case KEYCODE_BS:
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

void pad(std::string &s, int len,std::string t) {
    while(s.size() < len) {
        s.insert(0,t);
    }
}

size_t import_file(const std::string &filename,char* &data) {

    if(!file_exists(filename)) {
        print("import_file: file not found");
        return false;
    }

    std::ifstream image(filename,std::ios::binary);

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

bool export_file(const std::string &filename,char* bytes,size_t size) {

    std::ofstream image(filename,std::ios::binary);

    for(int i=0;i<size;i++) {
        image << bytes[i];
    }
    image.close();
    return true;
}

bool file_exists(std::string filename) {
    std::ifstream image(filename,std::ios::binary);
    return image.is_open();
}
