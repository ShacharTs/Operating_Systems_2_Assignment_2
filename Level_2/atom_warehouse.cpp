#include <iostream>
#include <string>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include <set>

#include "wareHouse.hpp"

#define PORT 12345
#define BUFFER_SIZE 1024

using namespace std;

int main() {
    int tcp_fd, udp_fd, new_socket, max_fd;
    struct sockaddr_in address{};
    fd_set master_fds, read_fds;
    char buffer[BUFFER_SIZE];
    int addrlen = sizeof(address);

    Warehouse warehouse;

    // Create TCP socket
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd < 0) {
        perror("TCP socket creation failed");
        return 1;
    }

    // Create UDP socket
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        perror("UDP socket creation failed");
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind TCP socket
    if (bind(tcp_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("TCP bind failed");
        return 1;
    }
    if (listen(tcp_fd, 10) < 0) {
        perror("TCP listen failed");
        return 1;
    }

    // Bind UDP socket
    if (bind(udp_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("UDP bind failed");
        return 1;
    }

    FD_ZERO(&master_fds);
    FD_SET(tcp_fd, &master_fds);
    FD_SET(udp_fd, &master_fds);
    max_fd = std::max(tcp_fd, udp_fd);

    cout << "Atom warehouse running on port " << PORT << endl;

    while (true) {
        read_fds = master_fds;
        if (select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr) < 0) {
            perror("select failed");
            break;
        }

        for (int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == tcp_fd) {
                    // New TCP client connection
                    new_socket = accept(tcp_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    if (new_socket < 0) {
                        perror("accept failed");
                        continue;
                    }
                    FD_SET(new_socket, &master_fds);
                    max_fd = std::max(max_fd, new_socket);
                    cout << "TCP client connected." << endl;
                } else if (i == udp_fd) {
                    // UDP message received
                    struct sockaddr_in client_addr{};
                    socklen_t len = sizeof(client_addr);
                    int n = recvfrom(udp_fd, buffer, BUFFER_SIZE - 1, 0,
                                     (struct sockaddr *)&client_addr, &len);
                    if (n > 0) {
                        buffer[n] = '\0';
                        string command(buffer);
                        cout << "Received UDP command: " << command << endl;

                        bool success = warehouse.processCommand(command);
                        string response = success ? "OK\n" : "ERROR\n";
                        sendto(udp_fd, response.c_str(), response.length(), 0,
                               (struct sockaddr *)&client_addr, len);

                        cout << "Responded to UDP client at "
                             << inet_ntoa(client_addr.sin_addr) << ":"
                             << ntohs(client_addr.sin_port) << " with " << response;
                    }
                } else {
                    // TCP client message
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

    close(tcp_fd);
    close(udp_fd);
    return 0;
}

