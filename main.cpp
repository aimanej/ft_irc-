#include "server.hpp"

// bool fd_exists(std::vector<Client *> clients)
// {
//     for(int t == 0; t < clients.size())
// }

int main(int ac, char **av)
{
    Server serv(av[1], atoi(av[2]));
    if(ac != 3)
    {
        std::cout << "no params added" << std::endl;
        exit(1);
    }
    
    std::cout << "Listening on 6667...\n";
    serv.run_serv();
}