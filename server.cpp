#include "server.hpp"
#include "channel.hpp"



void Server::create_channel(std::string name, Client *client)
{
    Channel new_channel(name,client);
    channels.push_back(&new_channel);
}