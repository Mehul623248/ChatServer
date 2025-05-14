#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

// Constants
const int SERVER_PORT = 12345;
const int MAX_BUFFER_SIZE = 512;

// Structure to hold client information
struct ClientInfo {
    SOCKET socket;
    std::string nickname; // You might want to add a nickname later
};

// Global vector to store connected clients and a mutex to protect it
std::vector<ClientInfo> clients;
std::mutex clientsMutex;

// Function to handle communication with a single client
void handleClient(ClientInfo client) {
    char buffer[MAX_BUFFER_SIZE];
    int bytesReceived;

    while ((bytesReceived = recv(client.socket, buffer, MAX_BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytesReceived] = '\0';
        std::string message = buffer;
        std::cout << "Received from client " << client.socket << ": " << message<< std::endl;

        // Broadcast the message to all other clients
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (const auto& otherClient : clients) {
            if (otherClient.socket != client.socket) {
                std::string outMsg = "Client " + std::to_string(client.socket) + " says: " + message;
                send(otherClient.socket, outMsg.c_str(), outMsg.length(), 0);
            }
        }
    }

    // Connection closed or error occurred
    std::cout << "Client " << client.socket << " disconnected." << std::endl;

    // Remove the client from the list
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (auto it = clients.begin(); it != clients.end(); ++it) {
            if (it->socket == client.socket) {
                closesocket(it->socket);
                clients.erase(it);
                break;
            }
        }
    }
}

int main() {
    WSADATA wsaData;
    SOCKET listenSocket = INVALID_SOCKET;
    sockaddr_in serverAddr;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    // Create the listening socket
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Bind the socket
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "bind failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Chat server started. Listening on port " << SERVER_PORT << "..." << std::endl;

    while (true) {
        
        sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
            continue; // Continue listening for other connections
        }

        std::cout << "Client connected: " << clientSocket << std::endl;

        // Add the new client to the list
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back({clientSocket, ""}); // Initially no nickname
        }

        // Create a new thread to handle communication with this client
        std::thread clientThread(handleClient, ClientInfo{clientSocket, ""});
        clientThread.detach(); // Let the thread run independently


      
    }

    // This part will not be reached in this simple infinite loop server
    closesocket(listenSocket);
    WSACleanup();

    return 0;
}