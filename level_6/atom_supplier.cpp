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
    cerr << "Usage: " << progname << " -h <hostname/IP> -p <port> <ATOM> <AMOUNT>\n"
         << "  -h, --host    Hostname or IP address of the atom‐warehouse server (required)\n"
         << "  -p, --port    TCP port on which the atom‐warehouse server is listening (required)\n"
         << "  <ATOM>        One of CARBON, OXYGEN, or HYDROGEN\n"
         << "  <AMOUNT>      Positive integer amount to add\n";
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

    // After flags, we expect exactly two more arguments: ATOM and AMOUNT
    if (optind + 2 != argc) {
        print_usage(argv[0]);
    }

    string atom   = argv[optind];
    string amount = argv[optind + 1];

    // Validate atom type
    set<string> valid_atoms = {"CARBON", "OXYGEN", "HYDROGEN"};
    if (valid_atoms.find(atom) == valid_atoms.end()) {
        cerr << "Invalid atom type. Valid atoms: CARBON, OXYGEN, HYDROGEN\n";
        return 1;
    }

    // Build the "ADD <ATOM> <AMOUNT>\n" message
    //string message = "ADD " + atom + " " + amount + "\n";
    string message = "ADD_ATOM " + atom + " " + amount + "\n";

    // Resolve hostname to IP
    hostent *server = gethostbyname(hostname.c_str());
    if (!server) {
        cerr << "Error: No such host: " << hostname << "\n";
        return 1;
    }

    // Resolve hostname using getaddrinfo
    struct addrinfo hints{}, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int err = getaddrinfo(hostname.c_str(), to_string(port).c_str(), &hints, &res);
    if (err != 0) {
        cerr << "getaddrinfo failed: " << gai_strerror(err) << "\n";
        return 1;
    }

    // Create a socket
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        perror("Socket creation failed");
        freeaddrinfo(res);
        return 1;
    }

    // Connect to the server
    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        perror("Connection failed");
        close(sock);
        freeaddrinfo(res);
        return 1;
    }

    freeaddrinfo(res);  // Clean up addrinfo


    // Send the command
    ssize_t sent = send(sock, message.c_str(), message.length(), 0);
    if (sent < 0) {
        perror("Send failed");
        close(sock);
        return 1;
    }

    // Receive the response
    char buffer[BUFFER_SIZE];
    int n = read(sock, buffer, BUFFER_SIZE - 1);
    if (n > 0) {
        buffer[n] = '\0';
        cout << "Server response: " << buffer;
    } else {
        cout << "No response received.\n";
    }

    close(sock);
    return 0;
}
