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
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include "wareHouse.hpp"

#define BUFFER_SIZE 1024

using namespace std;

// Defaults
int tcp_port            = 0;
int udp_port            = 0;
int timeout_secs        = 300;
int init_oxygen         = 0;
int init_carbon         = 0;
int init_hydrogen       = 0;
string uds_stream_path  = "";
string uds_datagram_path= "";

static void print_usage(const char *progname) {
    cout << "Usage: " << progname
         << " -T <tcp_port> -U <udp_port> [options]\n"
            "Options:\n"
            "  -T, --tcp-port <port>         TCP port to listen on (required)\n"
            "  -U, --udp-port <port>         UDP port to listen on (required)\n"
            "  -s, --stream-path <filepath>  UDS stream socket file path\n"
            "  -d, --datagram-path <filepath> UDS datagram socket file path\n"
            "  -o, --oxygen <count>          Initial number of oxygen atoms (default 0)\n"
            "  -c, --carbon <count>          Initial number of carbon atoms (default 0)\n"
            "  -h, --hydrogen <count>        Initial number of hydrogen atoms (default 0)\n"
            "  -t, --timeout <seconds>       Inactivity timeout in seconds (default 300)\n"
            "  --help                        Show this help message\n";
    exit(1);
}

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
}

int make_unix_stream_socket(const string &path) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
    unlink(path.c_str());
    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }
    if (listen(fd, 10) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

int make_unix_datagram_socket(const string &path) {
    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd < 0) return -1;
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
    unlink(path.c_str());
    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

int main(int argc, char *argv[]) {
    static struct option long_options[] = {
        {"tcp-port",    required_argument, 0, 'T'},
        {"udp-port",    required_argument, 0, 'U'},
        {"stream-path", required_argument, 0, 's'},
        {"datagram-path",required_argument,0, 'd'},
        {"oxygen",      required_argument, 0, 'o'},
        {"carbon",      required_argument, 0, 'c'},
        {"hydrogen",    required_argument, 0, 'h'},
        {"timeout",     required_argument, 0, 't'},
        {"help",        no_argument,       0,   0 },
        {0, 0, 0, 0}
    };

    bool saw_T = false, saw_U = false;
    int option_index = 0;
    int c;
    while ((c = getopt_long(argc, argv, "T:U:s:d:o:c:h:t:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'T':
                tcp_port = atoi(optarg);
                saw_T = true;
                break;
            case 'U':
                udp_port = atoi(optarg);
                saw_U = true;
                break;
            case 's':
                uds_stream_path = optarg;
                break;
            case 'd':
                uds_datagram_path = optarg;
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
            case 0:
                if (strcmp(long_options[option_index].name, "help") == 0) {
                    print_usage(argv[0]);
                }
                break;
            default:
                print_usage(argv[0]);
        }
    }

    if (!saw_T || !saw_U) {
        cerr << "Error: Both -T <tcp_port> and -U <udp_port> are required.\n";
        print_usage(argv[0]);
    }
    if (optind < argc) {
        cerr << "Error: Unexpected argument: " << argv[optind] << "\n";
        print_usage(argv[0]);
    }

    Warehouse warehouse;
    if (init_oxygen > 0) {
        if (!warehouse.addAtom("OXYGEN", init_oxygen))
            cerr << "Failed to add initial OXYGEN=" << init_oxygen << "\n";
    }
    if (init_carbon > 0) {
        if (!warehouse.addAtom("CARBON", init_carbon))
            cerr << "Failed to add initial CARBON=" << init_carbon << "\n";
    }
    if (init_hydrogen > 0) {
        if (!warehouse.addAtom("HYDROGEN", init_hydrogen))
            cerr << "Failed to add initial HYDROGEN=" << init_hydrogen << "\n";
    }

    thread inputThread(keyboardListener, ref(warehouse));

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

    sockaddr_in addr_in{};
    addr_in.sin_family = AF_INET;
    addr_in.sin_addr.s_addr = INADDR_ANY;
    addr_in.sin_port = htons(tcp_port);
    if (bind(tcp_fd, (sockaddr*)&addr_in, sizeof(addr_in)) < 0) {
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
    addr_in.sin_port = htons(udp_port);
    if (bind(udp_fd, (sockaddr*)&addr_in, sizeof(addr_in)) < 0) {
        perror("UDP bind failed");
        close(tcp_fd);
        close(udp_fd);
        return 1;
    }

    int uds_stream_fd = -1, uds_datagram_fd = -1;
    if (!uds_stream_path.empty()) {
        uds_stream_fd = make_unix_stream_socket(uds_stream_path);
        if (uds_stream_fd < 0) {
            perror("UDS stream bind/listen failed");
            // fallback: continue without UDS stream
        }
    }
    if (!uds_datagram_path.empty()) {
        uds_datagram_fd = make_unix_datagram_socket(uds_datagram_path);
        if (uds_datagram_fd < 0) {
            perror("UDS datagram bind failed");
            // fallback: continue without UDS datagram
        }
    }

    fd_set master_fds, read_fds;
    FD_ZERO(&master_fds);
    FD_SET(tcp_fd, &master_fds);
    FD_SET(udp_fd, &master_fds);
    int max_fd = max(tcp_fd, udp_fd);

    if (uds_stream_fd >= 0) {
        FD_SET(uds_stream_fd, &master_fds);
        max_fd = max(max_fd, uds_stream_fd);
    }
    if (uds_datagram_fd >= 0) {
        FD_SET(uds_datagram_fd, &master_fds);
        max_fd = max(max_fd, uds_datagram_fd);
    }

    cout << "Atom warehouse running on TCP port " << tcp_port
         << ", UDP port " << udp_port;
    if (uds_stream_fd >= 0)
        cout << ", UDS stream path " << uds_stream_path;
    if (uds_datagram_fd >= 0)
        cout << ", UDS datagram path " << uds_datagram_path;
    cout << ", timeout " << timeout_secs << " seconds.\n";

    while (true) {
        read_fds = master_fds;
        timeval tv{};
        tv.tv_sec = timeout_secs;
        tv.tv_usec = 0;

        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, &tv);
        if (activity < 0) {
            perror("select failed");
            break;
        }
        if (activity == 0) {
            cout << "No requests for " << timeout_secs << " seconds. Shutting down.\n";
            break;
        }

        for (int i = 0; i <= max_fd; ++i) {
            if (!FD_ISSET(i, &read_fds)) continue;

            // TCP
            if (i == tcp_fd) {
                sockaddr_in client_addr{};
                socklen_t addrlen = sizeof(client_addr);
                int new_socket = accept(tcp_fd, (sockaddr*)&client_addr, &addrlen);
                if (new_socket < 0) {
                    perror("accept failed");
                    continue;
                }
                FD_SET(new_socket, &master_fds);
                max_fd = max(max_fd, new_socket);
                cout << "TCP client connected.\n";
            }
            // UDP
            else if (i == udp_fd) {
                sockaddr_in client_addr{};
                socklen_t len = sizeof(client_addr);
                char buffer[BUFFER_SIZE];
                int n = recvfrom(udp_fd, buffer, BUFFER_SIZE - 1, 0,
                                 (sockaddr*)&client_addr, &len);
                if (n > 0) {
                    buffer[n] = '\0';
                    string command(buffer);
                    cout << "Received UDP command: " << command;
                    bool success = warehouse.processCommand(command);
                    string response = success ? "OK\n" : "ERROR\n";
                    sendto(udp_fd, response.c_str(), response.size(), 0,
                           (sockaddr*)&client_addr, len);
                    cout << "Responded to UDP client.\n";
                }
            }
            // UDS stream: new connection
            else if (uds_stream_fd >= 0 && i == uds_stream_fd) {
                sockaddr_un client_un{};
                socklen_t len = sizeof(client_un);
                int new_fd = accept(uds_stream_fd, (sockaddr*)&client_un, &len);
                if (new_fd < 0) {
                    perror("UDS stream accept failed");
                    continue;
                }
                FD_SET(new_fd, &master_fds);
                max_fd = max(max_fd, new_fd);
                cout << "UDS stream client connected.\n";
            }
            // UDS datagram
            else if (uds_datagram_fd >= 0 && i == uds_datagram_fd) {
                sockaddr_un client_un{};
                socklen_t len = sizeof(client_un);
                char buffer[BUFFER_SIZE];
                int n = recvfrom(uds_datagram_fd, buffer, BUFFER_SIZE - 1, 0,
                                 (sockaddr*)&client_un, &len);
                if (n > 0) {
                    buffer[n] = '\0';
                    string command(buffer);
                    cout << "Received UDS datagram command: " << command;
                    bool success = warehouse.processCommand(command);
                    string response = success ? "OK\n" : "ERROR\n";
                    sendto(uds_datagram_fd, response.c_str(), response.size(), 0,
                           (sockaddr*)&client_un, len);
                    cout << "Responded to UDS datagram client.\n";
                }
            }
            // Existing TCP or UDS-stream client sending data
            else {
                char buffer[BUFFER_SIZE];
                int valread = read(i, buffer, BUFFER_SIZE - 1);
                if (valread <= 0) {
                    close(i);
                    FD_CLR(i, &master_fds);
                    cout << "Client disconnected.\n";
                } else {
                    buffer[valread] = '\0';
                    string command(buffer);
                    cout << "Received command on fd " << i << ": " << command;
                    bool success = warehouse.processCommand(command);
                    string response = success ? "OK\n" : "ERROR\n";
                    send(i, response.c_str(), response.size(), 0);
                    cout << "Responded to client.\n";
                }
            }
        }
    }

    // Cleanup
    close(tcp_fd);
    close(udp_fd);
    if (uds_stream_fd >= 0) {
        close(uds_stream_fd);
        unlink(uds_stream_path.c_str());
    }
    if (uds_datagram_fd >= 0) {
        close(uds_datagram_fd);
        unlink(uds_datagram_path.c_str());
    }
    cin.setstate(std::ios::eofbit);
    inputThread.join();
    return 0;
}
