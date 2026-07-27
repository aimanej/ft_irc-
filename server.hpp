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
        std::vector<Channel *> channels;
    public:
        void create_channel(std::string name, Client *client);
};

#endif