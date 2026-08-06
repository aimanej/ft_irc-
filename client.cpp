#include "client.hpp"
#include "channel.hpp"

class Server;

Client::Client(int fd, char *ip, int port, Server *serv) : _fd(fd), _ip(ip), _port(port)
{
    info = {{"PASS", ""}, {"NICK", ""}, {"USER", ""}};
    registered = false;
    reg_entries = 0;
    linked = false;
    added = false;
    op = false;
    quit_request = false;
    failed_registration = false;
    this->server = serv;
    inv_cname = "";
}

Client::Client() : _fd(0), _ip(0), _port(0)
{
    info = {{"PASS", ""}, {"NICK", ""}, {"USER", ""}};
    registered = false;
    reg_entries = 0;
    linked = false;
    failed_registration = false;
    added = false;
    op = false;
    quit_request = false;

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
        quit_request = obj.quit_request;
        failed_registration = obj.failed_registration;
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

    std::stringstream ss(line);
    std::string command;
    ss >> command;
    if (command.size() > 3)
        this->cmd = command;

    int t = 0;
    while (ss)
    {
        std::string arg;
        ss >> arg;
        if (t == 1 && !arg.empty() && arg[0] == ':')
        {
            arg.erase(arg.begin());
        }

        if (!arg.empty())
            args.push_back(arg);
        t++;
    }
    this->command_hub();
    return 0;
}

int Client::registration()
{
    if (registered)
        return 0;

    std::string line = remove_nl();
    std::stringstream ss(line);
    std::string key, value;

    ss >> key;
    ss >> value;

    if (key == "CAP")
        return 0;
    std::map<std::string, std::string>::iterator it;
    it = info.find(key);
    if (it != info.end() && (it->second.size() == 0) && (value.size() >= 1) && (value.size() <= 10))
    {
        if (key == "NICK")
        {
            if (server->free_nickname(value) == false)
            {
                std::string msg = ":irc_server 433 * " + info["NICK"] + " :Nickname is already in use\r\n";
                send(_fd, msg.c_str(), msg.size(), 0);
                failed_registration = true;
                return 1;
            }
            else if (key == "NICK" && value[0] == '#')
            {
                std::string msg = ":irc_server 432 * " + info["NICK"] + " :Erroneous nickname\r\n";
                send(_fd, msg.c_str(), msg.size(), 0);
                failed_registration = true;
                return 1;
            }
        }

        else if (key == "PASS" && value != server->get_pwd())
        {
            send(_fd, "464", 3, 0);
            failed_registration = true;
            return 1;
        }
        info[key] = value;
        reg_entries++;
        if (reg_entries == 3)
        {
            registered = true;
        }
        return 0;
    }
    else
    {
        // std::cout << " registration failed " << std::endl;
        failed_registration = true;
        return 1;
    }

    // int t = 0;
    // return 0;
}

void Client::join_channel(Channel *chan)
{
    // chan->addClient();
}

bool Client::get_quit_req() const
{
    return quit_request;
}

bool Client::get_failed_reg() const
{
    return failed_registration;
}

void Client::command_hub()
{
    // std::cout << "hit the hub >> WITH CMD ::  [" << cmd << "]" << std::endl;
    if (cmd == "PRIVMSG")
    {
        if (args.size() >= 1 && args[0][0] == '#')
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
        // std::cout << "INSIDE QUIT FUNCTION " << std::endl;
        quit_request = true;
        std::string reason = (!args.empty()) ? args[0] : "Client Quit";
        std::string msg = "ERROR :Closing Link: " + info["NICK"] + " (Quit: " + reason + ")\r\n";
        send(_fd, msg.c_str(), msg.size(), 0);
    }
    else if (cmd == "INVITE" && args.size() >= 2)
    {
        server->invite_user(args[1], args[0], this);
    }
    else if (cmd == "LEAVE" && args.size() >= 1)
    {
        // server->leave_channel(args[0], this);
    }
    else if (cmd == "LIST")
    {
        // server->list_channels(this);
    }
    else if (cmd == "NICK")
    {
        if (args.empty())
        {
            std::string msg = ":irc_server 431 " + info["NICK"] + " :No nickname given\r\n";
            send(_fd, msg.c_str(), msg.size(), 0);
        }
        else if (!args.empty() && args[0][0] == '#')
        {
            std::string msg = ":irc_server 432 " + info["NICK"] + " " + args[0] + " :Erroneous nickname\r\n";
            send(_fd, msg.c_str(), msg.size(), 0);
        }
        else
            server->update_nick(this, args[0]);
    }
    else if (cmd == "PING")
    {
        std::string msg = "PONG ";
        if (!args.empty())
            msg += args[0] + "\r\n";
        else
            msg += "\r\n";
        send(_fd, msg.c_str(), msg.size(), 0);
    }
    else
    {
        // send(_fd, "command not recognized\n", 23, 0);
    }
    args.erase(args.begin(), args.end());
    cmd.erase(cmd.begin(), cmd.end());
    // cmd = "";
    // args.erase(args.begin(), args.end());
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

void Client::set_nick(std::string new_nick)
{
    info["NICK"] = new_nick;
}