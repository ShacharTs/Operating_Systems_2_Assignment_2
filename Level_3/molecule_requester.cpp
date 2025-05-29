#include <iostream>
#include <string>
#include <set>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 12345
#define BUFFER_SIZE 1024

using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <hostname> <MOLECULE_NAME> <AMOUNT>\n";
        cerr << "Valid molecules: WATER, CARBON DIOXIDE, ALCOHOL, GLUCOSE\n";
        return 1;
    }

    string hostname = argv[1];


    string molecule;
    for (int i = 2; i < argc - 1; ++i) {
        if (!molecule.empty()) molecule += " ";
        molecule += argv[i];
    }

    string amount = argv[argc - 1];

    set<string> valid_molecules = { "WATER", "CARBON DIOXIDE", "ALCOHOL", "GLUCOSE" };
    if (valid_molecules.find(molecule) == valid_molecules.end()) {
        cerr << "Invalid molecule type: '" << molecule << "'.\n";
        cerr << "Valid molecules: WATER, CARBON DIOXIDE, ALCOHOL, GLUCOSE\n";
        return 1;
    }

    string message = "DELIVER " + molecule + " " + amount;

    // Resolve hostname
    hostent *server = gethostbyname(hostname.c_str());
    if (!server) {
        cerr << "Error: No such host: " << hostname << endl;
        return 1;
    }

    // Create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    // Send the command
    ssize_t sent = sendto(sock, message.c_str(), message.size(), 0,
                          (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (sent < 0) {
        perror("sendto failed");
        close(sock);
        return 1;
    }

    cout << "Sent UDP command: " << message << endl;

    // Receive response
    char buffer[BUFFER_SIZE];
    sockaddr_in from_addr{};
    socklen_t from_len = sizeof(from_addr);
    ssize_t received = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0,
                                (struct sockaddr*)&from_addr, &from_len);

    if (received > 0) {
        buffer[received] = '\0';
        cout << "Server response: " << buffer;
    } else {
        cout << "No response received.\n";
    }

    close(sock);
    return 0;
}

