#include <iostream>
#include <string>
#include <set>
#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

using namespace std;

static void print_usage(const char* prog_name) {
    cerr << "Usage: " << prog_name << " -h <hostname/IP> -p <port> <MOLECULE_TYPE> <AMOUNT>\n"
         << "  -h, --host    Hostname or IP address of the atom‐warehouse server (required)\n"
         << "  -p, --port    UDP port on which the atom‐warehouse server is listening (required)\n"
         << "  <MOLECULE_TYPE>  One of: WATER, CARBON DIOXIDE, ALCOHOL, GLUCOSE\n"
         << "  <AMOUNT>      Positive integer amount to request\n";
    exit(1);
}

int main(int argc, char* argv[]) {
    string hostname;
    int port = 0;
    int opt;

    // Parse -h <hostname> and -p <port>
    while ((opt = getopt(argc, argv, "h:p:")) != -1) {
        switch (opt) {
            case 'h':
                hostname = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            default:
                print_usage(argv[0]);
        }
    }

    // Both -h and -p are mandatory
    if (hostname.empty() || port == 0) {
        cerr << "Error: Both -h <hostname/IP> and -p <port> are required.\n";
        print_usage(argv[0]);
    }

    // After flags, we expect at least two more arguments: MOLECULE_TYPE (which may be multi‐word) and AMOUNT
    if (optind + 2 > argc) {
        print_usage(argv[0]);
    }

    // Build the molecule name from argv[optind] .. argv[argc‐2]
    string molecule;
    for (int i = optind; i < argc - 1; ++i) {
        if (!molecule.empty()) molecule += " ";
        molecule += argv[i];
    }
    string amount = argv[argc - 1];

    // Validate molecule type
    set<string> valid_molecules = {
        "WATER",
        "CARBON DIOXIDE",
        "ALCOHOL",
        "GLUCOSE"
    };
    if (valid_molecules.find(molecule) == valid_molecules.end()) {
        cerr << "Error: Invalid molecule type: '" << molecule << "'.\n";
        cerr << "Valid molecules:\n"
             << "  WATER\n"
             << "  CARBON DIOXIDE\n"
             << "  ALCOHOL\n"
             << "  GLUCOSE\n";
        return 1;
    }

    // Build the "DELIVER <MOLECULE_TYPE> <AMOUNT>" message
    string command = "DELIVER " + molecule + " " + amount;

    // Prepare addrinfo for UDP
    struct addrinfo hints{}, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(hostname.c_str(), to_string(port).c_str(), &hints, &res) != 0) {
        perror("getaddrinfo failed");
        return 1;
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        perror("Socket creation failed");
        freeaddrinfo(res);
        return 1;
    }

    // Send the command
    ssize_t sent = sendto(sock,
                          command.c_str(),
                          command.size(),
                          0,
                          res->ai_addr,
                          res->ai_addrlen);
    if (sent < 0) {
        perror("sendto failed");
        freeaddrinfo(res);
        close(sock);
        return 1;
    }

    cout << "Sent UDP command: " << command << "\n";

    // Receive response
    char buffer[BUFFER_SIZE];
    struct sockaddr_storage sender_addr;
    socklen_t addr_len = sizeof(sender_addr);
    ssize_t n = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr*)&sender_addr, &addr_len);
    if (n > 0) {
        buffer[n] = '\0';
        cout << "Response: " << buffer;
    } else {
        cout << "No response received.\n";
    }

    freeaddrinfo(res);
    close(sock);
    return 0;
}
