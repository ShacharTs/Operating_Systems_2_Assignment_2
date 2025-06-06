#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <vector>
#include <algorithm>

#include "wareHouse.hpp"

#define PORT 12345
#define BUFFER_SIZE 1024

using namespace std;

int main() {
    int tcp_fd, new_socket, max_fd;
    struct sockaddr_in address{};
    fd_set master_fds, read_fds;
    char buffer[BUFFER_SIZE];
    int addrlen = sizeof(address);

    Warehouse warehouse;

    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(tcp_fd, (struct sockaddr *) &address, sizeof(address));
    listen(tcp_fd, 10);

    FD_ZERO(&master_fds);
    FD_SET(tcp_fd, &master_fds);
    max_fd = tcp_fd;

    cout << "Atom warehouse running on port " << PORT << endl;

    while (true) {

        read_fds = master_fds;
        select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);

        for (int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == tcp_fd) {
                    new_socket = accept(tcp_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen);
                    FD_SET(new_socket, &master_fds);
                    max_fd = max(max_fd, new_socket);
                    cout << "TCP client connected." << endl;
                } else {
                    int valread = read(i, buffer, BUFFER_SIZE - 1);
                    if (valread <= 0) {
                        close(i);
                        FD_CLR(i, &master_fds);
                        cout << "TCP client disconnected." << endl;
                    } else {
                        buffer[valread] = '\0';
                        string command(buffer);
                        bool success = warehouse.processCommand(command);
                        string response = success ? "OK\n" : "ERROR\n";
                        send(i, response.c_str(), response.length(), 0);
                    }
                }
            }
        }
    }

    return 0;
}
