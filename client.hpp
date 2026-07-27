#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <poll.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <map>
typedef enum {
    PASS,
    NICK,
    USER
} e_reg;

class Client{
    private:
        int _fd;
        char *_ip;
        int _port;
        std::string cmd;
        std::vector<std::string> args;
        std::map<std::string, std::string> info;
        std::string _buffer;
        bool registered;
        int reg_entries;
        bool linked;
        bool added;
        e_reg e;
        std::map<std::string, int> *name_list;
    public:
        Client();
        Client(int fd, char *ip, int port);
        Client &operator=(const Client &obj);

        void add_to_buffer(std::string str);
        void print_buffer();
        bool msg_complete();
        int parser();
        int registration(std::string line);
        void command_hub();
        // Client(const Client &obj);
        // ~Client();
        void link_list(std::map<std::string, int> *ptr);
        //getters
        int get_fd() const;
        bool get_reg_status() const;
        std::string get_nick() const;
        bool get_link() const;
        bool get_added() const;
        void set_added();
};


#endif