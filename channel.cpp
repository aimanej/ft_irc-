#include "channel.hpp"

Channel::Channel(const Channel &other)
{
    name = other.name;
    clients = other.clients;
    operators = other.operators;
}

Channel::Channel(std::string name, Client *creator)
{
    userlimit = 0;
    inviteOnly = false;
    topcrestricted = false;
    password = "";
    this->name = name;
    creator->set_operator(true);
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

bool Channel::getInviteOnly()
{
    return inviteOnly;
}

bool Channel::getTopcrestricted()
{
    return topcrestricted;
}

int Channel::addClient(Client *client)
{
    clients.push_back(client);
    return 1;
}

void Channel::removeClient(const std::string &nick)
{
    for (int i = 0; i < clients.size(); ++i)
    {
        if (clients[i]->get_nick() == nick)
        {
            clients.erase(clients.begin() + i);
            users.erase(nick);

            for (int j = 0; j < operators.size(); ++j)
            {
                if (operators[j]->get_nick() == nick)
                {
                    operators[j]->set_operator(false);
                    operators.erase(operators.begin() + j);
                    break;
                }
            }
            break;
        }
    }

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
        for (int i = 0; i < operators.size(); ++i)
        {
            if (operators[i] == client)
            {
                operators.erase(operators.begin() + i);
                break;
            }
        }
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
    for ( int t = 0; t < clients.size(); t++)
    {
        if(clients[t]->get_nick() == nick)
            return true;
    }
    return false;
}

bool Channel::operator_check(std::string nick)
{
    for (int t = 0; t < operators.size(); t++)
    {
        if (operators[t]->get_nick() == nick)
            return true;
    }
    return false;
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

void Channel::setPassword(const std::string newPassword)
{
    password = newPassword;
}

void Channel::setInviteOnly(bool inviteOnly)
{
    this->inviteOnly = inviteOnly;
}
void Channel::setTopcrestricted(bool topcrestricted)
{
    this->topcrestricted = topcrestricted;
}   
