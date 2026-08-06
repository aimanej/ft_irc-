#include "server.hpp"

// bool fd_exists(std::vector<Client *> clients)
// {
//     for(int t == 0; t < clients.size())
// }


bool isValidPort(char *arg)
{
    int i = 0;
    unsigned int portnum;
    std::string port = arg;
    std::stringstream sstr;

    while(i < port.length())
    {
        if(port[i] < '0' || port[i] > '9')
            return (false);
        i++;
    }
    sstr << port;
    sstr >> portnum;
    if(portnum < 1 || portnum > 65535)
        return false;
    return (true);
}


int main(int ac, char **av)
{
    if(ac != 3)
    {
        std::cout << "no params added" << std::endl;
        exit(1);
    }
    if(!isValidPort(av[1]))
    {
        std::cout << "unvalid port number" << std::endl;
        exit(1); 
    }

    Server serv(av[2], atoi(av[1]));

    std::cout << "Listening on " << av[1] << "...\n";
    serv.run_serv();
}

// /connect 127.0.0.1 6666 99