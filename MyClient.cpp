#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h> // For inet_pton
#include <string>
#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    SOCKET connectSocket = INVALID_SOCKET;
    sockaddr_in serverAddr;
    const int SIZE = 512;
    char sendBuffer[SIZE] = "";
    std::cin.getline(sendBuffer, SIZE);
    char recvBuffer[512];
    int recvResult;

    // 1. Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    // 2. Create the socket
    connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // 6. Connect to the server
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &(serverAddr.sin_addr)) <= 0) {
        std::cerr << "Invalid address/Address family not supported" << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    iResult = connect(connectSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (iResult == SOCKET_ERROR) {
        std::cerr << "connect failed: " << WSAGetLastError() << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    // 7. Send data to the server
    int sendResult = send(connectSocket, sendBuffer, strlen(sendBuffer), 0);
    if (sendResult == SOCKET_ERROR) {
        std::cerr << "send failed: " << WSAGetLastError() << std::endl;
    } else {
        std::cout << "Bytes sent: " << sendResult << std::endl;

        // Receive response from the server
        recvResult = recv(connectSocket, recvBuffer, sizeof(recvBuffer) - 1, 0);
        if (recvResult > 0) {
            recvBuffer[recvResult] = '\0';
            std::cout << "Received from server: " << recvBuffer << std::endl;
        } else if (recvResult == 0) {
            std::cout << "Connection closed by server." << std::endl;
        } else {
            std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
        }
    }

    // 8. Close the socket
    
    std::cin >> sendBuffer;
    if((strcmp(sendBuffer, "exit") == 0)) {
        std::cout << "Exiting..." << std::endl;
        closesocket(connectSocket);

    // 9. Clean up Winsock
        WSACleanup();
    }
  

    return 0;
}