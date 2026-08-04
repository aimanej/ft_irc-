#include "client.hpp"
#include "channel.hpp"

class Server;

Client::Client(int fd, char *ip, int port, Server *serv) : _fd(fd), _ip(ip), _port(port)
{
    info = {{"PASS", ""}, {"NICK", ""}, {"USER", ""}};
    registered = false;
    reg_entries = 0;
    name_list = NULL;
    linked = false;
    added = false;
    op = false;
    this->server = serv;
    inv_cname = "";
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
    op = false;
    inv_cname = "";
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
    // return info["NICK"];
}

std::string Client::remove_nl()
{
    int newline;
    std::string line;
    if ((newline = _buffer.find("\r\n")) >= 0)
    {
        line = _buffer.substr(0, newline);
        _buffer.erase(0, newline + 2);
    }
    else if ((newline = _buffer.find("\n")) >= 0)
    {
        line = _buffer.substr(0, newline);
        _buffer.erase(0, newline + 1);
    }
    return line;
}

int Client::parser()
{
    std::string line = remove_nl();
    // std::cout << "registration value : " << registered << std::endl;
    if (!registered)
    {
        // std::cout << "going through registration" << std::endl;
        if (registration(line))
            return 1;
        return 0;
    }
    std::stringstream ss(line);
    std::string command;
    ss >> command;
    if (command.size() > 3)
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
        if (key == "NICK" && (server->free_nickname(value) == false))
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
    // std::cout << "hit the hub >> WITH CMD ::  " << cmd << std::endl;
    if (cmd == "PVTMSG" && args.size() >= 2)
    {
        if(args[0][0] == '#')
            server->send_group_msg(args[0].substr(1, args[0].size()), args, this);
        else
            server->send_message(info["NICK"], args[0], args);
    }
    else if (cmd == "JOIN" && args.size() >= 1)
    {

        server->create_channel(args[0], this);
    }
    else if (cmd == "MODE")
    {
        this->server->set_mode(this, args);
    }
    else if (cmd == "TOPIC" && args.size() >= 1)
    {
        server->send_topic(args[0], args, this);
    }
    else if (cmd == "KICK" && args.size() >= 2)
    {
        server->kick_user(args[0], args[1], this);
    }
    else if (cmd == "QUIT")
    {
        send(_fd, "you have been disconnected\n", 27, 0);
        close(_fd);
    }
    else if (cmd == "INVITE" && args.size() >= 2)
    {
        server->invite_user(args[1], args[0], this);
    }
    else if (cmd == "LEAVE" && args.size() >= 1)
    {
        //server->leave_channel(args[0], this);
    }
    else if (cmd == "LIST")
    {
        // server->list_channels(this);
    }
    else
    {
        send(_fd, "command not recognized\n", 23, 0);
    }
    args.erase(args.begin(), args.end());
    cmd.erase(cmd.begin(), cmd.end());
    // cmd = "";
    // args.erase(args.begin(), args.end());
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

void Client::set_operator(bool is_op)
{
    op = is_op;
}

std::string Client::get_arg(int index)
{
    return this->args[index];
}

bool Client::is_operator()
{
    return op;
}

void Client::set_inv_cname(std::string cname)
{
    inv_cname = cname;
}

std::string Client::get_inv_cname()
{
    return inv_cname;
}