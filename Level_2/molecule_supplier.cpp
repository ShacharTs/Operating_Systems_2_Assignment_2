#include <iostream>
#include <string>
#include <set>
#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 1024

using namespace std;

void print_usage(const char* prog_name) {
    cerr << "Usage: " << prog_name << " <hostname> <MOLECULE_TYPE> <AMOUNT>" << endl;
    cerr << "Valid molecule types: WATER, CARBON DIOXIDE, ALCOHOL, GLUCOSE" << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        print_usage(argv[0]);
        return 1;
    }

    string hostname = argv[1];
    string molecule = argv[2];
    for (int i = 3; i < argc - 1; ++i) {
        molecule += " " + string(argv[i]);
    }
    string amount = argv[argc - 1];

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
