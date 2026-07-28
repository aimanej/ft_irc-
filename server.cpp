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
                channels[t]->addClient(client);
                channels[t]->add_user(client->get_nick(), client->get_fd());
                send(client->get_fd(), "channel joined\n", 15, 0);
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
