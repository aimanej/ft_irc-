#include "server.hpp"

// bool fd_exists(std::vector<Client *> clients)
// {
//     for(int t == 0; t < clients.size())
// }

int main(int ac, char **av)
{
    Server serv;
    if(ac != 3)
    {
        std::cout << "no params added" << std::endl;
        exit(1);
    }
    
    Server server;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6667);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 5);

    std::cout << "Listening on 6667...\n";

    std::vector<pollfd> poll_vec;
    pollfd ser_pol;
    ser_pol.fd = server_fd;
    ser_pol.events = POLLIN;
    ser_pol.revents = 0;

    poll_vec.push_back(ser_pol);
    std::vector<Client> clients;
    std::map<std::string, int> name_list;

    int count = 0;
    while (1)
    {
        int stat = poll(poll_vec.data(), poll_vec.size(), -1);
        if (stat <= 0)
            break;

        char buffer[1024];
        for (int t = 0; t <= count; t++)
        {
            if ((poll_vec[t].revents & POLLIN) && t == 0)
            {
                sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);
                int client_fd = accept(server_fd, (sockaddr *)&client_addr, &len);
                std::cout << "New connection accepted:\n";
                clients.push_back(Client(client_fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), &serv));
                pollfd pollinho;
                pollinho.fd = client_fd;
                pollinho.events = POLLIN;
                pollinho.revents = 0;
                poll_vec.push_back(pollinho);
                count++;
            }
            else if (poll_vec[t].revents & POLLIN)
            {
                int b = recv(poll_vec[t].fd, buffer, sizeof(buffer) - 1, 0);
                if (b <= 0)
                {
                    close(clients[t - 1].get_fd());
                    poll_vec.erase(poll_vec.begin() + t);
                    clients.erase(clients.begin() + (t - 1));
                    continue;
                }
                buffer[b] = '\0';
                std::cout << "recieved" << buffer << std::endl;
                clients[t - 1].add_to_buffer(buffer);
                clients[t - 1].link_list(&name_list);
                // clients[t - 1].print_buffer();
                while (clients[t - 1].msg_complete())
                {

                    if (clients[t - 1].parser())
                    {
                        // std::cout << "in here 11 >> " << std::endl;
                        close(clients[t - 1].get_fd());
                        poll_vec.erase(poll_vec.begin() + t);
                        clients.erase(clients.begin() + (t - 1));
                    }
                    if (clients[t - 1].get_reg_status() && !(clients[t - 1].get_added()))
                    {
                        if (name_list.find(clients[t - 1].get_nick()) != name_list.end())
                        {
                            send(clients[t - 1].get_fd(), "NICK already in use, you have been disconnected!\n", 49, 0);
                            close(clients[t - 1].get_fd());
                            poll_vec.erase(poll_vec.begin() + t);
                            name_list.erase(clients[t - 1].get_nick());
                            clients.erase(clients.begin() + (t - 1));
                        }
                        std::cout << "nickname for new user inside the map >> " << clients[t - 1].get_nick() << std::endl;
                        name_list.insert({clients[t - 1].get_nick(), clients[t - 1].get_fd()});
                        clients[t - 1].set_added();
                    }
                }
            }
        }
    }
    close(server_fd);
}