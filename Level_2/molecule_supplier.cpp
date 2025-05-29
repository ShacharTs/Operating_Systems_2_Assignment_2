#include <iostream>
#include <string>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include <set>

#define PORT 12345
#define BUFFER_SIZE 1024

using namespace std;

void print_usage(const char* prog_name) {
    cerr << "Usage: " << prog_name << " -h <hostname> -m <MOLECULE_TYPE> -n <AMOUNT>" << endl;
    cerr << "Valid molecule types: WATER, CARBON DIOXIDE, ALCOHOL, GLUCOSE" << endl;
}

int main(int argc, char* argv[]) {
    string hostname, molecule, amount = "1";

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];

        if (arg == "-h" && i + 1 < argc) {
            hostname = argv[++i];
        } else if (arg == "-m" && i + 1 < argc) {
            ++i;
            molecule = argv[i];
            // capture words until next flag or end
            while (i + 1 < argc && argv[i + 1][0] != '-') {
                molecule += " " + string(argv[++i]);
            }
        } else if (arg == "-n" && i + 1 < argc) {
            amount = argv[++i];
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (hostname.empty() || molecule.empty() || amount.empty()) {
        print_usage(argv[0]);
        return 1;
    }

    set<string> valid_molecules = {
        "WATER", "CARBON DIOXIDE", "ALCOHOL", "GLUCOSE"
    };

    if (!valid_molecules.count(molecule)) {
        cerr << "Error: Invalid molecule type '" << molecule << "'." << endl;
        print_usage(argv[0]);
        return 1;
    }

    string command = "DELIVER " + molecule + " " + amount;

    struct addrinfo hints{}, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(hostname.c_str(), to_string(PORT).c_str(), &hints, &res) != 0) {
        perror("getaddrinfo failed");
        return 1;
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        perror("Socket creation failed");
        freeaddrinfo(res);
        return 1;
    }

    sendto(sock, command.c_str(), command.length(), 0, res->ai_addr, res->ai_addrlen);
    cout << "Sent UDP command: " << command << endl;

    char buffer[BUFFER_SIZE];
    struct sockaddr_storage sender_addr;
    socklen_t addr_len = sizeof(sender_addr);
    ssize_t n = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr*)&sender_addr, &addr_len);
    if (n > 0) {
        buffer[n] = '\0';
        cout << "Response: " << buffer << endl;
    }

    freeaddrinfo(res);
    close(sock);
    return 0;
}

