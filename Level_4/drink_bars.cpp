#include <iostream>
#include <string>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include <set>
#include <thread>
#include <sstream>
#include <sys/time.h>
#include <getopt.h>
#include "wareHouse.hpp"

#define BUFFER_SIZE 1024

using namespace std;

// Defaults (will be overridden via command‐line; but -T and -U are mandatory)
int tcp_port      = 0;
int udp_port      = 0;
int timeout_secs  = 300;
int init_oxygen   = 0;
int init_carbon   = 0;
int init_hydrogen = 0;

/**
 * Prints usage information and exits.
 */
static void print_usage(const char *progname) {
    cout << "Usage: " << progname << " -T <tcp_port> -U <udp_port> [options]\n"
         << "Options:\n"
         << "  -T, --tcp-port <port>      TCP port to listen on (required)\n"
         << "  -U, --udp-port <port>      UDP port to listen on (required)\n"
         << "  -o, --oxygen <count>       Initial number of oxygen atoms (default 0)\n"
         << "  -c, --carbon <count>       Initial number of carbon atoms (default 0)\n"
         << "  -h, --hydrogen <count>     Initial number of hydrogen atoms (default 0)\n"
         << "  -t, --timeout <seconds>    Inactivity timeout in seconds (default 300)\n"
         << "  --help                     Show this help message\n";
    exit(1);
}

/**
 * Thread function that listens for keyboard input. Whenever the user types "GEN <molecule>",
 * it calls warehouse.maxDrinksPossible(<molecule>) and prints the result. Any other line
 * is forwarded to warehouse.processCommand().
 * The loop breaks when std::cin is set to EOF (e.g. by main() upon timeout).
 */
void keyboardListener(Warehouse& warehouse) {
    string line;
    while (getline(cin, line)) {
        istringstream iss(line);
        string command, name;
        if (!(iss >> command >> ws >> name)) {
            cerr << "Invalid input\n";
            continue;
        }

        if (command == "GEN") {
            uint64_t count = warehouse.maxDrinksPossible(name);
            cout << "Can generate " << count << " of " << name << endl;
        } else {
            warehouse.processCommand(line);
        }
    }
    // As soon as std::cin is in EOF state, getline() returns false and thread exits.
}

int main(int argc, char *argv[]) {
    // 1) Parse command‐line flags using getopt_long
    static struct option long_options[] = {
        {"tcp-port", required_argument, 0, 'T'},
        {"udp-port", required_argument, 0, 'U'},
        {"oxygen",   required_argument, 0, 'o'},
        {"carbon",   required_argument, 0, 'c'},
        {"hydrogen", required_argument, 0, 'h'},
        {"timeout",  required_argument, 0, 't'},
        {"help",     no_argument,       0,   0 },
        {0,          0,                 0,   0 }
    };

    bool saw_T = false;
    bool saw_U = false;
    int option_index = 0;

    while (true) {
        int c = getopt_long(argc, argv, "T:U:o:c:h:t:", long_options, &option_index);
        if (c == -1) break;  // no more flags

        switch (c) {
            case 'T':
                tcp_port = atoi(optarg);
                saw_T = true;
                break;
            case 'U':
                udp_port = atoi(optarg);
                saw_U = true;
                break;
            case 'o':
                init_oxygen = atoi(optarg);
                break;
            case 'c':
                init_carbon = atoi(optarg);
                break;
            case 'h':
                init_hydrogen = atoi(optarg);
                break;
            case 't':
                timeout_secs = atoi(optarg);
                break;
            case 0:  // long‐only options
                if (strcmp(long_options[option_index].name, "help") == 0) {
                    print_usage(argv[0]);
                }
                break;
            default:
                print_usage(argv[0]);
        }
    }

    // Enforce that both -T and -U were provided
    if (!saw_T || !saw_U) {
        cerr << "Error: Both -T <tcp_port> and -U <udp_port> are required.\n";
        print_usage(argv[0]);
    }

    // Reject any extra non-option arguments:
    if (optind < argc) {
        cerr << "Error: Unexpected argument: " << argv[optind] << "\n";
        print_usage(argv[0]);
    }

    // 2) Initialize the Warehouse and add initial atoms if requested
    Warehouse warehouse;
    if (init_oxygen > 0) {
        if (!warehouse.addAtom("OXYGEN", init_oxygen)) {
            cerr << "Failed to add initial OXYGEN=" << init_oxygen << "\n";
        }
    }
    if (init_carbon > 0) {
        if (!warehouse.addAtom("CARBON", init_carbon)) {
            cerr << "Failed to add initial CARBON=" << init_carbon << "\n";
        }
    }
    if (init_hydrogen > 0) {
        if (!warehouse.addAtom("HYDROGEN", init_hydrogen)) {
            cerr << "Failed to add initial HYDROGEN=" << init_hydrogen << "\n";
        }
    }

    // 3) Start the keyboard thread for GEN commands
    thread inputThread(keyboardListener, ref(warehouse));

    // 4) Create TCP & UDP sockets, bind to the chosen ports
    int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd < 0) {
        perror("TCP socket creation failed");
        return 1;
    }

    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        perror("UDP socket creation failed");
        close(tcp_fd);
        return 1;
    }

    // Bind TCP socket
    struct sockaddr_in address{};
    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = htons(tcp_port);

    if (bind(tcp_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("TCP bind failed");
        close(tcp_fd);
        close(udp_fd);
        return 1;
    }
    if (listen(tcp_fd, 10) < 0) {
        perror("TCP listen failed");
        close(tcp_fd);
        close(udp_fd);
        return 1;
    }

    // Bind UDP socket (reuse the same sockaddr_in but update port)
    address.sin_port = htons(udp_port);
    if (bind(udp_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("UDP bind failed");
        close(tcp_fd);
        close(udp_fd);
        return 1;
    }

    // 5) Enter the select(…) loop, using timeout_secs for inactivity
    fd_set master_fds, read_fds;
    FD_ZERO(&master_fds);
    FD_SET(tcp_fd, &master_fds);
    FD_SET(udp_fd, &master_fds);
    int max_fd = max(tcp_fd, udp_fd);

    cout << "Atom warehouse running on TCP port " << tcp_port
         << " and UDP port " << udp_port << " with timeout of " << timeout_secs << " seconds." << endl;

    while (true) {
        read_fds = master_fds;

        // Set up timeout for inactivity
        struct timeval tv;
        tv.tv_sec  = timeout_secs;
        tv.tv_usec = 0;

        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, &tv);
        if (activity < 0) {
            perror("select failed");
            break;
        }
        // If select() returns 0: no activity within timeout_secs
        if (activity == 0) {
            cout << "No requests for " << timeout_secs
                 << " seconds. Shutting down." << endl;
            goto cleanup;
        }

        // Handle all ready file descriptors
        for (int i = 0; i <= max_fd; i++) {
            if (!FD_ISSET(i, &read_fds)) continue;

            if (i == tcp_fd) {
                // New incoming TCP connection
                socklen_t addrlen = sizeof(address);
                int new_socket = accept(tcp_fd, (struct sockaddr *)&address, &addrlen);
                if (new_socket < 0) {
                    perror("accept failed");
                    continue;
                }
                FD_SET(new_socket, &master_fds);
                max_fd = max(max_fd, new_socket);
                cout << "TCP client connected." << endl;

            } else if (i == udp_fd) {
                // Incoming UDP datagram
                struct sockaddr_in client_addr{};
                socklen_t len = sizeof(client_addr);
                char buffer[BUFFER_SIZE];
                int n = recvfrom(udp_fd, buffer, BUFFER_SIZE - 1, 0,
                                 (struct sockaddr *)&client_addr, &len);
                if (n > 0) {
                    buffer[n] = '\0';
                    string command(buffer);
                    cout << "Received UDP command: " << command << endl;

                    bool success = warehouse.processCommand(command);
                    string response = success ? "OK\n" : "ERROR\n";
                    sendto(udp_fd, response.c_str(), response.size(), 0,
                           (struct sockaddr *)&client_addr, len);

                    cout << "Responded to UDP client at "
                         << inet_ntoa(client_addr.sin_addr) << ":"
                         << ntohs(client_addr.sin_port)
                         << " with " << response;
                }

            } else {
                // Data from an existing TCP client
                char buffer[BUFFER_SIZE];
                int valread = read(i, buffer, BUFFER_SIZE - 1);
                if (valread <= 0) {
                    // Client disconnected
                    close(i);
                    FD_CLR(i, &master_fds);
                    cout << "TCP client disconnected." << endl;
                } else {
                    buffer[valread] = '\0';
                    string command(buffer);
                    bool success = warehouse.processCommand(command);
                    string response = success ? "OK\n" : "ERROR\n";
                    send(i, response.c_str(), response.size(), 0);
                }
            }
        }
    }

cleanup:
    // Close TCP/UDP sockets
    close(tcp_fd);
    close(udp_fd);

    // Force std::cin into EOF state so keyboardListener exits immediately
    cin.setstate(std::ios::eofbit);

    // Wait for the keyboardListener thread to finish
    inputThread.join();
    return 0;
}
