#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

// Link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define BUFFER_SIZE 1024

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    // Create the socket
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Socket creation error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Set up the server address structure
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // FTP control port
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) != 1) {
        std::cerr << "Invalid address" << std::endl;
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    // Connect to the FTP server
    if (connect(sockfd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed: " << WSAGetLastError() << std::endl;
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    // Receive the server's welcome message
    char buffer[BUFFER_SIZE];
    int bytesReceived = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';  // Null-terminate the string
        std::cout << "Server: " << buffer << std::endl;
    }
    else {
        std::cerr << "Failed to receive welcome message: " << WSAGetLastError() << std::endl;
    }

    // Interactive loop: type FTP commands and send them to the server
    std::string userCommand;
    while (true) {
        std::cout << "FTP> ";
        std::getline(std::cin, userCommand);

        // Skip empty commands
        if (userCommand.empty())
            continue;

        // Append CRLF to the command as required by FTP protocol
        userCommand += "\r\n";

        // Send the command
        int sendResult = send(sockfd, userCommand.c_str(), static_cast<int>(userCommand.size()), 0);
        if (sendResult == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
            break;
        }

        // If the user typed QUIT, exit the loop after sending the command.
        if (userCommand.substr(0, 4) == "QUIT") {
            std::cout << "Closing connection." << std::endl;
            break;
        }

        // Wait for and print the server's response
        bytesReceived = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';  // Null-terminate the string
            std::cout << "Server: " << buffer << std::endl;
        }
        else if (bytesReceived == 0) {
            std::cout << "Connection closed by server." << std::endl;
            break;
        }
        else {
            std::cerr << "Receive failed: " << WSAGetLastError() << std::endl;
            break;
        }
    }

    // Close the connection and clean up Winsock
    closesocket(sockfd);
    WSACleanup();
    return 0;
}
