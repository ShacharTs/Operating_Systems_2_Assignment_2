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
         << "  -h, --host   Hostname or IP address of the atom‐warehouse server (required)\n"
         << "  -p, --port   UDP port on which the atom‐warehouse server is listening (required)\n"
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

    // Both -h and -p are mandatory
    if (hostname.empty() || port == 0) {
        cerr << "Error: Both -h <hostname/IP> and -p <port> are required.\n";
        print_usage(argv[0]);
    }

    // After flags, we expect at least two more arguments: MOLECULE_NAME (which may be multi‐word) and AMOUNT
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
        cerr << "Invalid molecule type: '" << molecule << "'.\n";
        cerr << "Valid molecules:\n"
             << "  WATER\n"
             << "  CARBON DIOXIDE\n"
             << "  ALCOHOL\n"
             << "  GLUCOSE\n";
        return 1;
    }

    // Build the "DELIVER <MOLECULE_NAME> <AMOUNT>" message
    string message = "DELIVER " + molecule + " " + amount;

    // Resolve hostname to IP
    hostent *server = gethostbyname(hostname.c_str());
    if (!server) {
        cerr << "Error: No such host: " << hostname << "\n";
        return 1;
    }

    // Create a UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(port);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    // Send the command
    ssize_t sent = sendto(sock,
                          message.c_str(),
                          message.size(),
                          0,
                          (struct sockaddr*)&server_addr,
                          sizeof(server_addr));
    if (sent < 0) {
        perror("sendto failed");
        close(sock);
        return 1;
    }

    cout << "Sent UDP command: " << message << "\n";

    // Receive response
    char buffer[BUFFER_SIZE];
    sockaddr_in from_addr{};
    socklen_t from_len = sizeof(from_addr);
    ssize_t received = recvfrom(sock,
                                buffer,
                                BUFFER_SIZE - 1,
                                0,
                                (struct sockaddr*)&from_addr,
                                &from_len);

    if (received > 0) {
        buffer[received] = '\0';
        cout << "Server response: " << buffer;
    } else {
        cout << "No response received.\n";
    }

    close(sock);
    return 0;
}
