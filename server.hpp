#ifndef SERVER_HPP
#define SERVER_HPP

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

#include "client.hpp"

class Channel;
class Client;

class Server{
    private:
        std::string pwd;
        int port;
        int socket_fd;
        std::vector<Channel *> channels;
        std::vector<pollfd> poll_vec;
        std::vector<Client> clients;
        std::map<std::string, int> name_list;
    public:
        Server(std::string, int port);
        void create_channel(std::string name, Client *client);
        ~Server();
        int get_sfd() const;
        pollfd *get_poll_data();
        size_t get_poll_size();
        void run_serv();
        void new_connection();
};

//to be done for server::
//OCF - constructor with pass and port.

#endif