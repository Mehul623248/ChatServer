#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h> // For inet_pton

#pragma comment(lib, "ws2_32.lib") // Link with the Winsock library

int main() {
    WSADATA wsaData;
    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET clientSocket = INVALID_SOCKET;
    sockaddr_in serverAddr;
    sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    char recvBuffer[512];
    int recvResult;

    // 1. Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    // 2. Create the listening socket
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // 3. Bind the socket
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces
    serverAddr.sin_port = htons(12345);     // Port number to listen on

    iResult = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (iResult == SOCKET_ERROR) {
        std::cerr << "bind failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // 4. Listen for incoming connections
    iResult = listen(listenSocket, SOMAXCONN); // SOMAXCONN suggests a reasonable maximum backlog
    if (iResult == SOCKET_ERROR) {
        std::cerr << "listen failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on port 12345..." << std::endl;

    // 5. Accept a client connection
    clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Client connected." << std::endl;

    // 7. Receive data from the client
    do {
        recvResult = recv(clientSocket, recvBuffer, sizeof(recvBuffer) - 1, 0);
        if (recvResult > 0) {
            recvBuffer[recvResult] = '\0'; // Null-terminate the received data
            std::cout << "Received from client: " << recvBuffer << std::endl;

            // Echo back the received data (optional)
            int sendResult = send(clientSocket, recvBuffer, recvResult, 0);
            if (sendResult == SOCKET_ERROR) {
                std::cerr << "send failed: " << WSAGetLastError() << std::endl;
                break;
            }
        } else if (recvResult == 0) {
            std::cout << "Connection closed by client." << std::endl;
        } else {
            std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
            break;
        }
    } while (recvResult > 0);

    // 8. Close the sockets
    closesocket(clientSocket);
    closesocket(listenSocket);

    // 9. Clean up Winsock
    WSACleanup();

    return 0;
}