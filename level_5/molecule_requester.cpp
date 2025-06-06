#include <iostream>
#include <string>
#include <set>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 1024

using namespace std;

static void print_usage(const char *progname) {
    cerr << "Usage: " << progname << " -h <hostname/IP> -p <port> <MOLECULE_NAME> <AMOUNT>\n"
         << "  -h, --host   Hostname or IP address of the atom-warehouse server (required)\n"
         << "  -p, --port   UDP port on which the atom-warehouse server is listening (required)\n"
         << "  <MOLECULE_NAME>  One of:\n"
         << "                     WATER\n"
         << "                     CARBON DIOXIDE\n"
         << "                     ALCOHOL\n"
         << "                     GLUCOSE\n"
         << "  <AMOUNT>     Positive integer amount to request\n";
    exit(1);
}

int main(int argc, char *argv[]) {
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

    // Validate hostname and port
    if (hostname.empty() || port <= 0) {
        cerr << "Error: Both -h <hostname/IP> and -p <port> are required.\n";
        print_usage(argv[0]);
    }

    // Validate remaining arguments: molecule name + amount
    if (optind + 2 > argc) {
        print_usage(argv[0]);
    }

    // Combine molecule name parts (might be multi-word)
    string molecule;
    for (int i = optind; i < argc - 1; ++i) {
        if (!molecule.empty()) molecule += " ";
        molecule += argv[i];
    }
    string amount = argv[argc - 1];

    // Validate molecule name
    set<string> valid_molecules = {
        "WATER", "CARBON DIOXIDE", "ALCOHOL", "GLUCOSE"
    };
    if (valid_molecules.find(molecule) == valid_molecules.end()) {
        cerr << "Invalid molecule type: '" << molecule << "'.\n"
             << "Valid molecules: WATER, CARBON DIOXIDE, ALCOHOL, GLUCOSE\n";
        return 1;
    }

    // Compose message
    string message = "DELIVER " + molecule + " " + amount;

    // Use getaddrinfo to resolve the hostname
    struct addrinfo hints{}, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int err = getaddrinfo(hostname.c_str(), to_string(port).c_str(), &hints, &res);
    if (err != 0) {
        cerr << "getaddrinfo: " << gai_strerror(err) << "\n";
        return 1;
    }

    // Create UDP socket
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        perror("Socket creation failed");
        freeaddrinfo(res);
        return 1;
    }

    // Send UDP message
    ssize_t sent = sendto(sock, message.c_str(), message.length(), 0, res->ai_addr, res->ai_addrlen);
    if (sent < 0) {
        perror("sendto failed");
        freeaddrinfo(res);
        close(sock);
        return 1;
    }

    cout << "Sent UDP command: " << message << "\n";

    // Receive response
    char buffer[BUFFER_SIZE];
    sockaddr_storage from_addr{};
    socklen_t from_len = sizeof(from_addr);
    ssize_t received = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0, (sockaddr*)&from_addr, &from_len);

    if (received > 0) {
        buffer[received] = '\0';
        cout << "Server response: " << buffer;
    } else {
        cout << "No response received.\n";
    }

    freeaddrinfo(res);
    close(sock);
    return 0;
}
