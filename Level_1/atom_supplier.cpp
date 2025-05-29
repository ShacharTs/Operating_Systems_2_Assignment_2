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
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <hostname> <ATOM> <AMOUNT>\n";
        return 1;
    }

    string hostname = argv[1];
    string atom = argv[2];
    string amount = argv[3];

    set<string> valid_atoms = {"CARBON", "OXYGEN", "HYDROGEN"};
    if (valid_atoms.find(atom) == valid_atoms.end()) {
        cerr << "Invalid atom type. Valid atoms: CARBON, OXYGEN, HYDROGEN\n";
        return 1;
    }

    string message = "ADD " + atom + " " + amount + "\n";

    // Resolve hostname
    hostent *server = gethostbyname(hostname.c_str());
    if (!server) {
        cerr << "Error: No such host: " << hostname << endl;
        return 1;
    }

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    // Connect
    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    // Send command
    send(sock, message.c_str(), message.length(), 0);

    // Receive response
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
