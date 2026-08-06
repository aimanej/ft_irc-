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
#include <string>

#include "server.hpp"

class Channel;
class Server;
typedef enum {
    PASS,
    NICK,
    USER
} e_reg;


class Client{
    private:
        Server *server;

        int _fd;
        char *_ip;
        int _port;
        std::string cmd;
        std::vector<std::string> args;
        std::map<std::string, std::string> info;
        std::string inv_cname;
        std::string _buffer;
        bool registered;
        int reg_entries;
        bool linked;
        bool added;
        bool op;
        bool quit_request;
        bool failed_registration;
        e_reg e;

    public:
        Client();
        Client(int fd, char *ip, int port, Server *serv);
        Client &operator=(const Client &obj);

        void join_channel(Channel *chan);
        void add_to_buffer(std::string str);
        void print_buffer();
        bool msg_complete();
        int parser();
        int registration();
        void command_hub();
        // Client(const Client &obj);
        // ~Client();
        //getters
        int get_fd() const;
        bool get_reg_status() const;
        std::string get_nick() const;
        bool get_link() const;
        bool get_added() const;
        bool get_failed_reg() const;
        void set_added();
        void set_operator(bool is_op);
        std::string remove_nl();
        std::string get_arg(int index);
        bool is_operator();
        void set_inv_cname(std::string cname);
        std::string get_inv_cname() ;
        bool get_quit_req() const;
        void set_nick(std::string new_nick);
        // friend void Server::create_channel(std::string name, Client *client);
};


#endif