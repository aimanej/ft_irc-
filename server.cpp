#include "server.hpp"
#include "channel.hpp"

Server::Server(std::string pass, int port) : pwd(pass), port(port)
{
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6667);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(socket_fd, 5);

    pollfd ser_pol;
    ser_pol.fd = socket_fd;
    ser_pol.events = POLLIN;
    ser_pol.revents = 0;

    poll_vec.push_back(ser_pol);
}

pollfd *Server::get_poll_data()
{
    // pollfd *fds = ;
    return (poll_vec.data());
}

size_t Server::get_poll_size()
{
    return poll_vec.size();
}

int Server::get_sfd() const
{
    return socket_fd;
}

Server::~Server()
{
    close(socket_fd);
}

void Server::new_connection()
{
    sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int client_fd = accept(socket_fd, (sockaddr *)&client_addr, &len);

    std::cout << "New connection accepted:\n";
    clients.push_back(new Client(client_fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), this));

    pollfd pollinho;
    pollinho.fd = client_fd;
    pollinho.events = POLLIN;
    pollinho.revents = 0;
    poll_vec.push_back(pollinho);
}

int Server::read_from_socket(int t)
{
    char buffer[1024];
    int b = recv(poll_vec[t].fd, buffer, sizeof(buffer) - 1, 0);
    if (b <= 0)
    {
        close(clients[t - 1]->get_fd());
        poll_vec.erase(poll_vec.begin() + t);
        name_list.erase(clients[t - 1]->get_nick());
        clients.erase(clients.begin() + (t - 1));
        return 1;
    }
    buffer[b] = '\0';
    std::cout << "recieved" << buffer << std::endl;
    clients[t - 1]->add_to_buffer(buffer);
    clients[t - 1]->link_list(&name_list);
    return 0;
}

void Server::process_completed_message(int t)
{
    while (clients[t - 1]->msg_complete())
    {

        if (clients[t - 1]->parser())
        {
            // std::cout << "in here 11 >> " << std::endl;
            close(clients[t - 1]->get_fd());
            poll_vec.erase(poll_vec.begin() + t);
            clients.erase(clients.begin() + (t - 1));
        }
        if (clients[t - 1]->get_reg_status() && !(clients[t - 1]->get_added()))
        {
            if (name_list.find(clients[t - 1]->get_nick()) != name_list.end())
            {
                send(clients[t - 1]->get_fd(), "NICK already in use, you have been disconnected!\n", 49, 0);
                close(clients[t - 1]->get_fd());
                poll_vec.erase(poll_vec.begin() + t);
                name_list.erase(clients[t - 1]->get_nick());
                clients.erase(clients.begin() + (t - 1));
            }
            std::cout << "nickname for new user inside the map >> " << clients[t - 1]->get_nick() << std::endl;
            name_list.insert({clients[t - 1]->get_nick(), clients[t - 1]->get_fd()});
            clients[t - 1]->set_added();
        }
    }
}
void Server::run_serv()
{
    // int count = 0;
    while (1)
    {
        int stat = poll(poll_vec.data(), poll_vec.size(), -1);
        if (stat <= 0)
            break;

        for (int t = 0; t < poll_vec.size(); t++)
        {
            if ((poll_vec[t].revents & POLLIN) && t == 0)
            {
                new_connection();
            }
            else if (poll_vec[t].revents & POLLIN)
            {
                if (read_from_socket(t))
                    continue;
                process_completed_message(t);
            }
        }
    }
}

void Server::send_message(std::string sender, std::string recipient, std::vector<std::string> msgs)
{
    std::cout << "inside pvt msgfunction" << std::endl;
    std::map<std::string, int>::iterator it = name_list.find(recipient);

    std::cout << "looking for : " << recipient << std::endl;
    if (it != name_list.end())
    {
        std::cout << "sending to : " << it->first << std::endl;
        send(it->second, sender.c_str(), sender.size(), 0);
        send(it->second, ": ", 2, 0);
        for (int t = 1; t < msgs.size(); t++)
        {

            send(it->second, msgs[t].c_str(), msgs[t].size(), 0);
            send(it->second, " ", 1, 0);
        }
        send(it->second, "\n", 1, 0);
    }
    else
    {
        send(name_list[sender], "user not found", 14, 0);
    }
}

bool Server::free_nickname(std::string nick)
{
    return (name_list.find(nick) == name_list.end()) ? true : false;
}

void Server::create_channel(std::string name, Client *client)
{
    std::cout << " nae enteredd ::::::  " << name << std::endl;
    if (name[0] != '#')
    {
        send(client->get_fd(), "use # before the name of a channel you would like to join\n", 58, 0);
        return;
    }
    std::string cname = name.substr(1, name.size());
    std::cout << "name of wanted to be created  --> " << cname << std::endl;

    if (open_channels.find(cname) == open_channels.end())
    {
        channels.push_back(new Channel(cname, client));
        open_channels.insert(cname);
        send(client->get_fd(), "channel created and joined successfully\n", 40, 0);
    }
    else
    {
        for (int t = 0; t < channels.size(); t++)
        {
            if (channels[t]->getName() == cname && !(channels[t]->user_check(client->get_nick())))
            {
                if (channels[t]->getuserlimit() == 0 || channels[t]->getClientCount() < channels[t]->getuserlimit())
                {
                    if (channels[t]->getInviteOnly() && client->get_inv_cname() != cname)
                    {
                        send(client->get_fd(), "channel is invite only, cannot join\n", 36, 0);
                        return;
                    }
                    channels[t]->addClient(client);
                    channels[t]->add_user(client->get_nick(), client->get_fd());
                    send(client->get_fd(), "Joined channel #", 15, 0);
                    send(client->get_fd(), cname.c_str(), cname.size(), 0);
                    send(client->get_fd(), "\n", 1, 0);
                }
                else 
                    send(client->get_fd(), "channel is full, cannot join\n", 30, 0);
            }
            else if (channels[t]->getName() == cname && (channels[t]->user_check(client->get_nick())))
                send(client->get_fd(), "you are already part of this channel\n", 37, 0);
        }
    }
    // Channel new_channel(name, client);
}

void Server::send_group_msg(std::string cname, std::vector<std::string> args, Client *client)
{
    for(int t = 0; t < channels.size(); t++)
    {
        if(channels[t]->getName() == cname)
        {
            channels[t]->send_msg(args, client);
        }
    }
}

void Server::set_mode(Client *client, std::vector<std::string> args)
{
    std::string cname;
    if (args.size() >= 1 && args[0][0] == '#')
        cname = args[0].substr(1, args[0].size());
    else
    {
        send(client->get_fd(), "use # before the name of a channel you would like to join\n", 58, 0);
        return;
    }

    bool found = false;
    for(int t = 0; t < channels.size(); t++)
    {
        if(channels[t]->getName() == cname)
        {
            found = true;
            if (args.size() < 2)
            {
                send(client->get_fd(), "no mode specified\n", 18, 0);
                return;
            }
            if (channels[t]->operator_check(client->get_nick()))
            {
                Client *target = NULL;
                if (args[1] == "+o" || args[1] == "-o")
                {
                    if (args.size() >= 3)
                    {
                        for (int i = 0; i < clients.size(); i++)
                        {
                            if (clients[i]->get_nick() == args[2])
                            {
                                target = clients[i];
                                break;
                            }
                        }
                    }
                    else
                    {
                        send(client->get_fd(), "no target specified for operator mode change\n", 46, 0);
                        return;
                    }
                }
                if(args[1] == "+i")
                    channels[t]->setInviteOnly(true);
                else if(args[1] == "-i")
                    channels[t]->setInviteOnly(false);
                else if(args[1] == "+t")
                    channels[t]->setTopcrestricted(true);
                else if(args[1] == "-t")
                    channels[t]->setTopcrestricted(false);
                else if(args[1] == "+k" && args.size() >= 3)
                    channels[t]->setPassword(args[2]);
                else if(args[1] == "-k")
                    channels[t]->setPassword("");
                else if(args[1] == "+l" && args.size() >= 3)
                    channels[t]->setuserlimit(std::stoi(args[2]));
                else if(args[1] == "-l")
                    channels[t]->setuserlimit(0);
                else if(args[1] == "+o" && target != NULL)
                    channels[t]->setOperator(target, true);
                else if(args[1] == "-o" && target != NULL)
                    channels[t]->setOperator(target, false);
                else if ((args[1] == "+o" || args[1] == "-o") && target == NULL)
                    send(client->get_fd(), "target user not found\n", 22, 0);
            }
            else
                send(client->get_fd(), "you are not an operator of this channel\n", 39, 0);
            return;
        }
    }

    if (!found)
        send(client->get_fd(), "no such channel\n", 17, 0);
}

void Server::send_topic(std::string cname, std::vector<std::string> args, Client *client)
{
    (void)cname;

    if (args[0][0] != '#')
    {
        send(client->get_fd(), "use # before the name of a channel you would like to join\n", 58, 0);
        return;
    }
    args[0] = args[0].substr(1, args[0].size());
    bool found = false;
    for(int t = 0; t < channels.size(); t++)
    {
        if(channels[t]->getName() == args[0])
        {
            found = true;
            if(args.size() >= 2)
            {
                std::string ntopic;
                for (int i = 1; i < args.size(); i++)
                {
                    if (!ntopic.empty())
                        ntopic += " ";
                    ntopic += args[i];
                }
                if (channels[t]->getTopcrestricted() && !(channels[t]->operator_check(client->get_nick())))
                {
                    send(client->get_fd(), "you are not an operator of this channel\n", 39, 0);
                    return;
                }
                channels[t]->setTopic(ntopic);
                send(client->get_fd(), channels[t]->getTopic().c_str(), channels[t]->getTopic().size(), 0);
            }
            else
                send(client->get_fd(), channels[t]->getTopic().c_str(), channels[t]->getTopic().size(), 0);
            send(client->get_fd(), "\n", 1, 0);
        }
    }
    if (!found)
        send(client->get_fd(), "no such channel\n", 17, 0);
    
}




void Server::kick_user(std::string cname, std::string nick, Client *client)
{
    if (cname[0] != '#')
    {
        send(client->get_fd(), "use # before the name of a channel you would like to join\n", 58, 0);
        return;
    }
    cname = cname.substr(1, cname.size());

    bool found = false;
    for(int t = 0; t < channels.size(); t++)
    {
        if(channels[t]->getName() == cname)
        {
            found = true;
            if(channels[t]->user_check(nick))
            {
                if (channels[t]->operator_check(client->get_nick()))
                {
                    channels[t]->removeClient(nick);
                    send(client->get_fd(), "user kicked from channel\n", 26, 0);
                    send(name_list[nick], "you have been kicked from channel\n", 34, 0);
                }
                else
                    send(client->get_fd(), "you are not an operator of this channel\n", 39, 0);
            }
            else
                send(client->get_fd(), "user not found in channel\n", 28, 0);
            return;
        }
    }

    if (!found)
        send(client->get_fd(), "no such channel\n", 17, 0);
}

void Server::invite_user(std::string cname, std::string nick, Client *client)
{
    if (cname[0] != '#')
    {
        send(client->get_fd(), "use # before the name of a channel you would like to join\n", 58, 0);
        return;
    }
    cname = cname.substr(1, cname.size());

    bool found = false;
    for(int t = 0; t < channels.size(); t++)
    {
        if(channels[t]->getName() == cname)
        {
            found = true;
            if(channels[t]->user_check(nick))
            {
                send(client->get_fd(), "user already in channel\n", 25, 0);
                return;
            }
            if (name_list.find(nick) == name_list.end())
            {
                send(client->get_fd(), "user not found\n", 15, 0);
                return;
            }
            if (channels[t]->operator_check(client->get_nick()))
            {
                for (int i = 0; i < clients.size(); i++)
                {
                    if (clients[i]->get_nick() == nick)
                    {
                        clients[i]->set_inv_cname(cname);
                        break;
                    }
                }
                send(name_list[nick], "you have been invited to channel\n", 33, 0);
                send(name_list[nick], cname.c_str(), cname.size(), 0);
                send(name_list[nick], "\n", 1, 0);
                send(client->get_fd(), "user invited to channel\n", 25, 0);
            }
            else
                send(client->get_fd(), "you are not an operator of this channel\n", 39, 0);
            return;
        }
    }

    if (!found)
        send(client->get_fd(), "no such channel\n", 17, 0);
}
