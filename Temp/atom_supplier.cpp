#include <iostream>
#include <string>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include <set>

#define PORT 12345

using namespace std;

void print_usage(const char* prog_name) {
    cerr << "Usage: " << prog_name << " -h <hostname> -t <ATOM_TYPE> -n <AMOUNT>" << endl;
    cerr << "Valid atom types: CARBON, OXYGEN, HYDROGEN" << endl;
}

int main(int argc, char* argv[]) {
    string hostname, atom, amount;
    int opt;

    while ((opt = getopt(argc, argv, "h:t:n:")) != -1) {
        switch (opt) {
            case 'h': hostname = optarg; break;
            case 't': atom = optarg; break;
            case 'n': amount = optarg; break;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    if (hostname.empty() || atom.empty() || amount.empty()) {
        print_usage(argv[0]);
        return 1;
    }

    set<string> valid_atoms = {"CARBON", "OXYGEN", "HYDROGEN"};
    if (!valid_atoms.count(atom)) {
        cerr << "Error: Invalid atom type '" << atom << "'." << endl;
        print_usage(argv[0]);
        return 1;
    }

    // Resolve hostname using getaddrinfo
    struct addrinfo hints{}, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

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

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        perror("Connection failed");
        freeaddrinfo(res);
        close(sock);
        return 1;
    }

    freeaddrinfo(res);

    string msg = "ADD " + atom + " " + amount + "\n";
    send(sock, msg.c_str(), msg.length(), 0);
    cout << "Sent command: " << msg;

    close(sock);
    return 0;
}
