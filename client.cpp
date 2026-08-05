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
    if (!registered)
    {
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

    if(key == "CAP")
        return 0;
    std::map<std::string, std::string>::iterator it;
    it = info.find(key);
    if (it != info.end() && (it->second.size() == 0) && (value.size() >= 1) && (value.size() <= 10))
    {
        if ((key == "NICK" && (server->free_nickname(value) == false)) || key == "NICK" && value[0] == '#')
        {
            send(_fd, "nickname format not accepted or already in use\n", 47, 0);
            return 1;
        }
        else if(key == "PASS" && value != server->get_pwd())
        {
            send(_fd, "464", 3, 0);
            return 1;
        }
        info[key] = value;
        reg_entries++;
        if (reg_entries == 3)
        {
            std::string msg = ":irc_server 001 " + this->info["NICK"] + " :Welcome to the IRC server Network\r\n";

            // std::string msg = "welcome to the irc server: 001 " + this->info["NICK"] + "\r\n";
            registered = true;
            // send(_fd, "irc_server: ", 12, 0);
            send(_fd, msg.c_str(), msg.size(), 0);
            // send(_fd, this->info["NICK"].c_str(), this->info["NICK"].size(), 0);
            // send(_fd, " :Welcome to the IRC server Network\r\n", 36, 0);
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
    if (cmd == "PRIVMSG" && args.size() >= 2)
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
    else if(cmd == "PING")
    {
        send(_fd, "PONG irc_server\r\n", 17, 0);
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