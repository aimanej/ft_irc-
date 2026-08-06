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
#include <set>

#include "client.hpp"

class Channel;
class Client;

class Server{
    private:
        std::string pwd;
        int port;
        int socket_fd;
        std::vector<Channel *> channels;
        std::set<std::string> open_channels;
        std::vector<pollfd> poll_vec;
        std::vector<Client *> clients;
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
        int read_from_socket(int poll_index);
        void process_completed_message(int poll_index);
        bool free_nickname(std::string nick);
        std::string get_pwd() const;
        void send_group_msg(std::string cname, std::vector<std::string> args, Client *client);
        void send_message(std::string sender, std::string recipient, std::vector<std::string> msgs);
        void set_mode(Client *client, std::vector<std::string> args);
        void send_topic(std::string cname, std::vector<std::string> args, Client *client);
        void kick_user(std::string cname, std::string nick, Client *client);
        void invite_user(std::string cname, std::string nick, Client *client);
        void leave_channel(std::string cname, Client *client);
        void remove_client(int t);
        void update_nick(Client *client, std::string new_ick);
};

//to be done for server::
//OCF - constructor with pass and port.

#endif