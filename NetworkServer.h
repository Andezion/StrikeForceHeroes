#pragma once

#include <thread>
#include <atomic>
#include <vector>

struct _ENetHost;

class NetworkServer
{
public:
    explicit NetworkServer(uint16_t port = 1234);
    ~NetworkServer();

    bool start();
    void stop();
    void broadcast(const std::vector<uint8_t>& data);

private:
    void serviceLoop();

    _ENetHost* host_ = nullptr;
    uint16_t port_;
    std::thread thread_;

    std::atomic<bool> running_{false};
};
