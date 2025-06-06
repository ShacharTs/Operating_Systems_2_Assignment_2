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

    struct addrinfo hints{}, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // Use AF_UNSPEC for IPv4/6 support
    hints.ai_socktype = SOCK_STREAM;

    int err = getaddrinfo(hostname.c_str(), to_string(PORT).c_str(), &hints, &res);
    if (err != 0) {
        cerr << "getaddrinfo: " << gai_strerror(err) << endl;
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
        close(sock);
        freeaddrinfo(res);
        return 1;
    }

    freeaddrinfo(res);


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
