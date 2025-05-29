#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <cstdint>
#include <vector>

#include "ChemistryEnums.hpp"

#define PORT 12345
#define BUFFER_SIZE 1024

using namespace std;

enum class Action {
    ADD,
    DELIVER,
    GEN_SORT_DRINK,
    GEN_VODKA,
    GEN_CHAMPAGNE,
    UNKNOWN
};

unordered_map<AtomElement, uint64_t> atomCounts = {
    {AtomElement::CARBON, 0},
    {AtomElement::HYDROGEN, 0},
    {AtomElement::OXYGEN, 0}
};

unordered_map<MoleculeType, uint64_t> moleculeCounts = {
    {MoleculeType::WATER, 0},
    {MoleculeType::CARBON_DIOXIDE, 0},
    {MoleculeType::ALCOHOL, 0},
    {MoleculeType::GLUCOSE, 0}
};

const unordered_map<string, AtomElement> StrToAtomElement = {
    {"CARBON", AtomElement::CARBON},
    {"OXYGEN", AtomElement::OXYGEN},
    {"HYDROGEN", AtomElement::HYDROGEN}
};

const unordered_map<AtomElement, string> AtomElementToStr = {
    {AtomElement::CARBON, "CARBON"},
    {AtomElement::OXYGEN, "OXYGEN"},
    {AtomElement::HYDROGEN, "HYDROGEN"}
};

const unordered_map<string, MoleculeType> StrToMoleculeType = {
    {"WATER", MoleculeType::WATER},
    {"CARBON DIOXIDE", MoleculeType::CARBON_DIOXIDE},
    {"ALCOHOL", MoleculeType::ALCOHOL},
    {"GLUCOSE", MoleculeType::GLUCOSE}
};

const unordered_map<MoleculeType, string> MoleculeTypeToStr = {
    {MoleculeType::WATER, "WATER"},
    {MoleculeType::CARBON_DIOXIDE, "CARBON DIOXIDE"},
    {MoleculeType::ALCOHOL, "ALCOHOL"},
    {MoleculeType::GLUCOSE, "GLUCOSE"}
};


unordered_map<MoleculeType, unordered_map<AtomElement, uint64_t> > moleculeRecipes = {
    {MoleculeType::WATER, {{AtomElement::HYDROGEN, 2}, {AtomElement::OXYGEN, 1}}},
    {MoleculeType::CARBON_DIOXIDE, {{AtomElement::CARBON, 1}, {AtomElement::OXYGEN, 2}}},
    {MoleculeType::ALCOHOL, {{AtomElement::CARBON, 2}, {AtomElement::HYDROGEN, 6}, {AtomElement::OXYGEN, 1}}},
    {MoleculeType::GLUCOSE, {{AtomElement::CARBON, 6}, {AtomElement::HYDROGEN, 12}, {AtomElement::OXYGEN, 6}}}
};

Action parse_action(const string &action_str) {
    if (action_str == "ADD") return Action::ADD;
    if (action_str == "DELIVER") return Action::DELIVER;
    return Action::UNKNOWN;
}

bool process_command(const string &command) {
    istringstream iss(command);
    string actionStr;
    if (!(iss >> actionStr)) return false;

    vector<string> parts;
    string word;
    while (iss >> word) parts.push_back(word);

    if (parts.size() < 2) return false;

    int64_t amount;
    try {
        amount = stoll(parts.back());
    } catch (...) {
        return false;
    }
    parts.pop_back();

    string nameStr = parts[0];
    for (size_t i = 1; i < parts.size(); ++i)
        nameStr += " " + parts[i];

    Action action = parse_action(actionStr);

    switch (action) {
        case Action::ADD: {
            auto it = StrToAtomElement.find(nameStr);
            if (it == StrToAtomElement.end()) return false;
            AtomElement atom = it->second;
            if (atomCounts[atom] > UINT64_MAX - static_cast<uint64_t>(amount)) return false;
            atomCounts[atom] += static_cast<uint64_t>(amount);
            break;
        }
        case Action::DELIVER: {
            auto it = StrToMoleculeType.find(nameStr);
            if (it == StrToMoleculeType.end()) return false;
            MoleculeType molecule = it->second;
            const auto &recipe = moleculeRecipes[molecule];
            for (const auto &[atom, qty] : recipe) {
                if (atomCounts[atom] < qty * amount) return false;
            }
            for (const auto &[atom, qty] : recipe)
                atomCounts[atom] -= qty * amount;
            moleculeCounts[molecule] += amount;
            break;
        }
        default:
            return false;
    }

    cout << "Inventory: ";
    for (const auto &[atom, count] : atomCounts)
        cout << AtomElementToStr.at(atom) << "=" << count << " ";
    for (const auto &[molecule, count] : moleculeCounts)
        cout << MoleculeTypeToStr.at(molecule) << "=" << count << " ";
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
