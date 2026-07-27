#include "client.hpp"
#include "channel.hpp"


Client::Client(int fd, char *ip, int port) : _fd(fd), _ip(ip), _port(port)
{
    info = {{"PASS", ""}, {"NICK", ""}, {"USER", ""}};
    registered = false;
    reg_entries = 0;
    name_list = NULL;
    linked = false;
    added = false;
    // got_pass = false;
    // got_nick = false;
    // got_user = false;
}

Client::Client() : _fd(0), _ip(0), _port(0)
{
    info = {{"PASS", ""}, {"NICK", ""}, {"USER", ""}};
    registered = false;
    reg_entries = 0;
    name_list = NULL;
    linked = false;
    added = false;
    // got_pass = false;
    // got_nick = false;
    // got_user = false;
}

Client &Client::operator=(const Client &obj)
{
    if (this != &obj)
    {
        _ip = obj._ip;
        _fd = obj._fd;
        _port = obj._port;
        info = obj.info;
        reg_entries = obj.reg_entries;
        linked = obj.linked;
    }
    return *this;
}

void Client::add_to_buffer(std::string str)
{
    _buffer = _buffer + str;
}

void Client::print_buffer()
{
    std::cout << " was inside the clients buffer >>>> " << _buffer << std::endl;
}

bool Client::msg_complete()
{
    if ((_buffer.find('\n') != std::string::npos) || (_buffer.find("\r\n") != std::string::npos))
        return 1;
    return 0;
}

int Client::get_fd() const
{
    return _fd;
}

bool Client::get_reg_status() const
{
    return registered;
}

std::string Client::get_nick() const
{
    std::string str("NICK");
    std::map<std::string, std::string>::const_iterator it;
    it = info.find(str);
    return it->second;
}

int Client::parser()
{
    int newline = _buffer.find('\n');
    std::string line = _buffer.substr(0, newline);
    _buffer.erase(0, newline + 1);

    // std::cout << "registration value : " << registered << std::endl;
    if (!registered)
    {
        // std::cout << "going through registration" << std::endl;
        if (registration(line))
            return 1;
    }

    if(linked)
        std::cout << "sie of the map pointer >>>>>>>>>>>>. " << name_list->size() << std::endl;

    std::stringstream ss(line);
    std::string command;
    ss >> command;
    if(command.size() > 3)
        this->cmd = command;
        
    while (ss)
    {
        std::string arg;
        ss >> arg;
        if (!arg.empty())
            args.push_back(arg);
    }
    this->command_hub();
    return 0;
}

int Client::registration(std::string line)
{
    std::stringstream ss(line);
    std::string key, value;

    ss >> key;
    ss >> value;

    std::map<std::string, std::string>::iterator it;
    it = info.find(key);
    if (it != info.end() && (it->second.size() == 0) && (value.size() >= 1) && (value.size() <= 10))
    {
        if(key == "NICK" && (name_list->find(value) != name_list->end()))
        {
            send(_fd, "NICK already in use, you have been disconnected!\n", 49, 0);
            return 1;
        }
        info[key] = value;
        reg_entries++;
        if (reg_entries == 3)
        {
            registered = true;
            send(_fd, "you are now registered !\n", 25, 0);
        }
        return 0;
    }
    else
    {
        std::cout << " registration failed " << std::endl;
        return 1;
    }

    int t = 0;
    return 0;
}

void Client::join_channel(Channel *chan)
{
    // chan->addClient();
}

void Client::command_hub()
{
    std::cout << "hit the hub >> WITH CMD ::  " << cmd << std::endl;
    if(cmd == "PVTMSG" && args.size() >= 3)
    {
        std::cout << "inside pvt msgfunction" << std::endl;
        std::map<std::string , int>::iterator it = name_list->find(args.at(0));

        if(it != name_list->end())
        {
            for(int t = 0; t < args.size(); t++)
            {
                
                send(it->second, args[t].c_str(), args[t].size(), 0);
                send(it->second, " " , 1, 0);
            }
        }
        else
        {
            send(_fd, "user not found", 14, 0);
        }
    }
    else if (cmd == "JOIN" && args.size() >= 2)
    {
        // verify if channel name got #

        // join_channel(Channel(args[1], ))
        std::cout << args[1] << std::endl;

    }
    cmd = "";
    args.erase(args.begin(), args.end());
}


void Client::link_list(std::map<std::string, int> *ptr)
{
    name_list = ptr;
}

bool Client::get_link() const
{
    return linked;
}
bool Client::get_added() const
{
    return added;
}

void Client::set_added()
{
    added = true;
}