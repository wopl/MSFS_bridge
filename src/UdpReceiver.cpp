#include "UdpReceiver.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

UdpReceiver::UdpReceiver(int port, PacketHandler handler)
    : port(port), handler(handler), running(false) {
    std::cout << "[UDP-RECV] UdpReceiver constructed for port " << port << std::endl << std::flush;
}

UdpReceiver::~UdpReceiver() {
    stop();
}

void UdpReceiver::start() {
    std::cout << "[UDP-RECV] start() called" << std::endl << std::flush;
    if (running) return;
    running = true;
    recvThread = std::thread(&UdpReceiver::receiveLoop, this);
}

void UdpReceiver::stop() {
    running = false;
    if (recvThread.joinable()) recvThread.join();
}

void UdpReceiver::receiveLoop() {
    std::cout << "[UDP-RECV] receiveLoop started" << std::endl << std::flush;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return;
    }
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        WSACleanup();
        return;
    }
    std::cout << "[UDP-RECV] Socket created successfully" << std::endl;
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        closesocket(sock);
        WSACleanup();
        return;
    }
    std::cout << "[UDP-RECV] Socket bound to port " << port << std::endl;
    char buf[512];
    while (running) {
        int len = recv(sock, buf, sizeof(buf)-1, 0);
        if (len > 0) {
            buf[len] = '\0';
            std::cout << "[UDP-RECV] Received packet of length " << len << std::endl;
            handler(std::string(buf, len));
        } else if (len == 0) {
            std::cout << "[UDP-RECV] Received empty UDP packet" << std::endl;
        } else {
            std::cerr << "[UDP-RECV] recv() error: " << WSAGetLastError() << std::endl;
        }
    }
    closesocket(sock);
    WSACleanup();
    std::cout << "[UDP-RECV] receiveLoop exiting" << std::endl;
}
