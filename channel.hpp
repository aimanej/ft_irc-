#ifndef CAHNNEL_HPP
#define CHANNEL_HPP


#include "client.hpp"

class Channel
{
    private:
        std::string name;
        std::string topic;
        std::string password;

        int userlimit;


        std::vector<Client *> clients;
        std::vector<Client *> operators;
        std::map<std::string, int> users;

        std::string getMode();

        // bool isOperator;
        bool inviteOnly;
        bool topcrestricted;
         
        
    public:
        Channel(const Channel &other);
        Channel(std::string name, Client *creator);
        ~Channel();
        
        std::string getTopic();
        std::string getPassword();
        std::string getName();
        int         getClientCount();
        int         getuserlimit(); 
        bool getInviteOnly();
        bool getTopcrestricted();


        void add_user(std::string nick, int fder);
        int addClient(Client *client);
        
        bool operator_check(std::string nick);
        bool user_check(std::string nick);
        
        void send_msg(std::vector<std::string> args, Client *client);
        void removeClient(const std::string &nick);
        
        
        // void broadcast(Client *sender, const std::string &message);

        void setTopic(const std::string newTopic);
        void setOperator(Client *client, bool isOperator);
        void setuserlimit(int limit);
        void setPassword(const std::string newPassword);
        void setInviteOnly(bool inviteOnly);
        void setTopcrestricted(bool topcrestricted); 
        // bool areoperator(Client *client);
};

#endif