#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include <cstdint>
#include <vector>

#include "../AthomType.hpp"

#define PORT 12345
#define BUFFER_SIZE 1024

using namespace std;

enum class Action {
    ADD,
    DELIVER,
    UNKNOWN
};

unordered_map<AtomType, string> atomToStr = {
    {AtomType::CARBON, "CARBON"},
    {AtomType::OXYGEN, "OXYGEN"},
    {AtomType::HYDROGEN, "HYDROGEN"},
    {AtomType::WATER, "WATER"},
    {AtomType::H20, "H20"},
    {AtomType::CARBON_DIOXIDE, "CARBON DIOXIDE"},
    {AtomType::ALCOHOL, "ALCOHOL"},
    {AtomType::GLUCOSE, "GLUCOSE"}
};

unordered_map<string, AtomType> strToAtom = {
    {"CARBON", AtomType::CARBON},
    {"OXYGEN", AtomType::OXYGEN},
    {"HYDROGEN", AtomType::HYDROGEN},
    {"WATER", AtomType::WATER},
    {"H20", AtomType::H20},
    {"CARBON DIOXIDE", AtomType::CARBON_DIOXIDE},
    {"ALCOHOL", AtomType::ALCOHOL},
    {"GLUCOSE", AtomType::GLUCOSE}
};

unordered_map<AtomType, uint64_t> atom_counts = {
    {AtomType::CARBON, 0},
    {AtomType::OXYGEN, 0},
    {AtomType::HYDROGEN, 0},
    {AtomType::WATER, 0},
    {AtomType::CARBON_DIOXIDE, 0},
    {AtomType::ALCOHOL, 0},
    {AtomType::GLUCOSE, 0}
};

unordered_map<AtomType, unordered_map<AtomType, uint64_t> > molecule_recipes = {
    {AtomType::WATER, {{AtomType::HYDROGEN, 2}, {AtomType::OXYGEN, 1}}},
    {AtomType::CARBON_DIOXIDE, {{AtomType::CARBON, 1}, {AtomType::OXYGEN, 2}}},
    {AtomType::ALCOHOL, {{AtomType::CARBON, 2}, {AtomType::HYDROGEN, 6}, {AtomType::OXYGEN, 1}}},
    {AtomType::GLUCOSE, {{AtomType::CARBON, 6}, {AtomType::HYDROGEN, 12}, {AtomType::OXYGEN, 6}}}
};

Action parse_action(const string &action_str) {
    if (action_str == "ADD") return Action::ADD;
    if (action_str == "DELIVER") return Action::DELIVER;
    return Action::UNKNOWN;
}

bool process_command(const string &command) {
    istringstream iss(command);
    string actionStr;
    if (!(iss >> actionStr)) {
        cerr << "Error: Could not parse action." << endl;
        return false;
    }

    string word;
    vector<string> parts;
    while (iss >> word)
        parts.push_back(word);

    if (parts.size() < 2) {
        cerr << "Error: Not enough arguments." << endl;
        return false;
    }

    int64_t amount;
    try {
        amount = stoll(parts.back());
    } catch (...) {
        cerr << "Error: Invalid amount." << endl;
        return false;
    }
    parts.pop_back();

    string atomStr = parts[0];
    for (size_t i = 1; i < parts.size(); ++i)
        atomStr += " " + parts[i];


    auto it = strToAtom.find(atomStr);
    if (it == strToAtom.end()) {
        cerr << "Error: Unknown atom/molecule type." << endl;
        return false;
    }

    AtomType atom = it->second;
    Action action = parse_action(actionStr);

    switch (action) {
        case Action::ADD:
            if (atom_counts[atom] > UINT64_MAX - static_cast<uint64_t>(amount)) {
                cerr << "Error: Overflow." << endl;
                return false;
            }
            atom_counts[atom] += static_cast<uint64_t>(amount);
            break;
        case Action::DELIVER:
            if (!molecule_recipes.count(atom)) {
                cerr << "Error: Cannot deliver unknown molecule." << endl;
                return false;
            }
            for (const auto &[ingredient, qty]: molecule_recipes[atom]) {
                if (atom_counts[ingredient] < qty * amount) {
                    cerr << "Error: Not enough ingredients." << endl;
                    return false;
                }
            }
            for (const auto &[ingredient, qty]: molecule_recipes[atom])
                atom_counts[ingredient] -= qty * amount;

            atom_counts[atom] += amount;
            break;
        default:
            cerr << "Error: Unknown action." << endl;
            return false;
    }

    cout << "Inventory: ";
    for (const auto &[type, count]: atom_counts)
        cout << atomToStr[type] << "=" << count << " ";
    cout << endl;

    return true;
}

int main() {
    int tcp_fd, udp_fd, new_socket, max_fd;
    struct sockaddr_in address{};
    fd_set master_fds, read_fds;
    char buffer[BUFFER_SIZE];
    int addrlen = sizeof(address);

    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(tcp_fd, (struct sockaddr *) &address, sizeof(address));
    listen(tcp_fd, 10);

    bind(udp_fd, (struct sockaddr *) &address, sizeof(address));

    FD_ZERO(&master_fds);
    FD_SET(tcp_fd, &master_fds);
    FD_SET(udp_fd, &master_fds);
    max_fd = max(tcp_fd, udp_fd);

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
                } else if (i == udp_fd) {
                    struct sockaddr_in client_addr;
                    socklen_t len = sizeof(client_addr);
                    int n = recvfrom(udp_fd, buffer, BUFFER_SIZE - 1, 0,
                                     (struct sockaddr *) &client_addr, &len);
                    if (n > 0) {
                        buffer[n] = '\0';
                        string command(buffer);
                        cout << "Received UDP command: " << command << endl;

                        bool success = process_command(command);
                        string response = success ? "OK\n" : "ERROR\n";
                        sendto(udp_fd, response.c_str(), response.length(), 0,
                               (struct sockaddr *) &client_addr, len);

                        cout << "Responded to UDP client at "
                                << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port)
                                << " with " << response;
                    }
                } else {
                    int valread = read(i, buffer, BUFFER_SIZE - 1);
                    if (valread <= 0) {
                        close(i);
                        FD_CLR(i, &master_fds);
                        cout << "TCP client disconnected." << endl;
                    } else {
                        buffer[valread] = '\0';
                        string command(buffer);
                        bool success = process_command(command);
                        string response = success ? "OK\n" : "ERROR\n";
                        send(i, response.c_str(), response.length(), 0);
                    }
                }
            }
        }
    }

    return 0;
}
