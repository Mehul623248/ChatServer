#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

// Constants
const int SERVER_PORT = 12345;
const int MAX_BUFFER_SIZE = 512;
const char* SERVER_IP = "127.0.0.1"; // Change this to the server's IP if it's not local

// Function to receive messages from the server
void receiveMessages(SOCKET serverSocket) {
    char buffer[MAX_BUFFER_SIZE];
    int bytesReceived;

    while ((bytesReceived = recv(serverSocket, buffer, MAX_BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytesReceived] = '\0';
        std::cout << "Server: " << buffer << std::endl;
    }

    std::cout << "Connection to server closed." << std::endl;
}

int main() {
    WSADATA wsaData;
    SOCKET connectSocket = INVALID_SOCKET;
    sockaddr_in serverAddr;
    std::string message;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    // Create the socket
    connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Prepare the server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &(serverAddr.sin_addr)) <= 0) {
        std::cerr << "Invalid server address." << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    // Connect to the server
    if (connect(connectSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "connect failed: " << WSAGetLastError() << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to the chat server." << std::endl;

    // Start a separate thread to receive messages from the server
    std::thread receiveThread(receiveMessages, connectSocket);

    // Main loop for sending messages
    while (true) {
        std::getline(std::cin, message);
        if (message == "/quit") {
            break;
        }

        if (send(connectSocket, message.c_str(), message.length(), 0) == SOCKET_ERROR) {
            std::cerr << "send failed: " << WSAGetLastError() << std::endl;
            break;
        }
    }

    // Clean up
    closesocket(connectSocket);
    WSACleanup();
    receiveThread.join(); // Wait for the receiving thread to finish

    return 0;
}