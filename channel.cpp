#include "channel.hpp"

Channel::Channel(const Channel &other)
{
    name = other.name;
    clients = other.clients;
    operators = other.operators;
}

Channel::Channel(std::string name, Client *creator)
{
    this->name = name;
    clients.push_back(creator);
    operators.push_back(creator);
    users.insert({creator->get_nick(), creator->get_fd()});
    std::cout << "operator " << operators[0]->get_nick() << " joined" << std::endl;
}

Channel::~Channel()
{
    clients.clear();
    operators.clear();
}

std::string Channel::getTopic()
{
    return topic;
}

std::string Channel::getPassword()
{
    return password;
}

std::string Channel::getName()
{
    return name;
}

int Channel::getClientCount()
{
    return clients.size();
}

int Channel::getuserlimit()
{
    return userlimit;
}

void Channel::addClient(Client *client)
{
    if (userlimit > 0 && clients.size() >= userlimit)
    {
        std::cout << "Cannot add client: user limit reached." << std::endl;
        return;
    }
    if (clients.empty())
    {
        clients.push_back(client);
        operators.push_back(client);
        return;
    }
    clients.push_back(client);
}

void Channel::removeClient(Client *client)
{
    //
}


// void Channel::broadcast(Client *sender, const std::string &message)
// {
//     for (Client *client : clients)
//     {
//         // if (client != sender)
//         // {
//             // send message to everyone (client part) instead of printing
//             std::cout << client->get_nick() << " Broadcasting message from #"<< this->getName() << ": " << message << std::endl;
//         // }
//     }
// }

void Channel::setTopic(const std::string newTopic)
{
    topic = newTopic;
}

void Channel::setOperator(Client *client, bool isOperator)
{
    if (isOperator)
    {
        operators.push_back(client);
    }
    else
    {
        // Remove client from operators vector
    }
}

void Channel::setuserlimit(int limit)
{
    userlimit = limit;
}

void Channel::add_user(std::string name, int fder)
{
    users.insert({name, fder});
}

bool Channel::user_check(std::string nick)
{
    return (users.find(nick) == users.end()) ? false : true;
}

void Channel::send_msg( std::vector<std::string> args, Client *client)
{
    for(std::map<std::string, int>::iterator it = users.begin(); it != users.end(); it++)
    {
        send(it->second, "#", 1, 0);
        send(it->second, name.c_str(), name.size(), 0);
        send(it->second, " ", 1, 0);
        send(it->second, client->get_nick().c_str(), client->get_nick().size(), 0);
        send(it->second, ": ", 1, 0);
        for(int t = 1; t < args.size(); t++)
        {
            send(it->second, args[t].c_str(), args[t].size(), 0);
            send(it->second, " ", 1, 0);
        }
    }
}