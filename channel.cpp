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


void Channel::broadcast(Client *sender, const std::string &message)
{
    for (Client *client : clients)
    {
        // if (client != sender)
        // {
            // send message to everyone (client part) instead of printing
            std::cout << client->get_nick() << " Broadcasting message from #"<< this->getName() << ": " << message << std::endl;
        // }
    }
}

void Channel::setTopic(const std::string &newTopic)
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