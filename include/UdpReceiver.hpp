#pragma once
#include <thread>
#include <atomic>
#include <functional>
#include <string>

class UdpReceiver {
public:
    using PacketHandler = std::function<void(const std::string&)>;
    UdpReceiver(int port, PacketHandler handler);
    ~UdpReceiver();
    void start();
    void stop();
private:
    void receiveLoop();
    int port;
    PacketHandler handler;
    std::thread recvThread;
    std::atomic<bool> running;
};
