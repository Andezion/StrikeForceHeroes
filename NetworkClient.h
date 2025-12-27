#pragma once

#include <thread>
#include <atomic>
#include <vector>
#include <functional>
#include <string>

struct ENetHost;
struct ENetPeer;

class NetworkClient
{
public:
    NetworkClient();
    ~NetworkClient();

    bool connectTo(const std::string& host, uint16_t port);
    void disconnect();
    void send(const std::vector<uint8_t>& data) const;
    void setReceiveCallback(std::function<void(const std::vector<uint8_t>&)> cb);

private:
    void serviceLoop();

    ENetHost* client_ = nullptr;
    ENetPeer* peer_ = nullptr;
    std::thread thread_;

    std::atomic<bool> running_{false};
    std::function<void(const std::vector<uint8_t>&)> callback_;
};
