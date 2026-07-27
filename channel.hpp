#ifndef CAHNNEL_HPP
#define CHANNEL_HPP

#include "client.hpp"
class Client;

class Channel
{
    private:
        std::string name;
        std::string topic;
        std::string password;

        int userlimit;


        std::vector<Client *> clients;
        std::vector<Client *> operators;

        std::string getMode();

        // bool isOperator;
        
    public:
        Channel(const Channel &other);
        Channel(std::string name, Client *creator);
        ~Channel();
        
        std::string getTopic();
        std::string getPassword();
        std::string getName();
        int getClientCount();
        int getuserlimit(); 

        void addClient(Client *client);
        void removeClient(Client *client);
        
        
        
        void broadcast(Client *sender, const std::string &message);

        void setTopic(const std::string &newTopic);
        void setOperator(Client *client, bool isOperator);
        void setuserlimit(int limit);
        // bool areoperator(Client *client);
};

#endif