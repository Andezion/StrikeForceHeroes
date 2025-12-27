#include "NetworkClient.h"
#include <iostream>

#include <enet/enet.h>

NetworkClient::NetworkClient() = default;

NetworkClient::~NetworkClient()
{
    disconnect();
}

bool NetworkClient::connectTo(const std::string& host, const uint16_t port)
{
    if (running_)
    {
        return true;
    }

    if (enet_initialize() != 0)
    {
        std::cerr << "ENet initialization failed\n";
        return false;
    }

    client_ = enet_host_create(nullptr, 1, 2, 0, 0);
    if (!client_)
    {
        std::cerr << "Failed to create ENet client host\n";
        enet_deinitialize();

        return false;
    }

    ENetAddress address;
    enet_address_set_host(&address, host.c_str());
    address.port = port;

    peer_ = enet_host_connect(client_, &address, 2, 0);
    if (!peer_)
    {
        std::cerr << "No available peers for initiating an ENet connection\n";
        enet_host_destroy(client_);

        client_ = nullptr;
        enet_deinitialize();
        return false;
    }

    ENetEvent event;
    if (enet_host_service(client_, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
    {
        std::cout << "Connection to " << host << ":" << port << " succeeded.\n";
    }
    else
    {
        enet_peer_reset(peer_);
        std::cerr << "Connection to " << host << ":" << port << " failed.\n";

        enet_host_destroy(client_);
        client_ = nullptr;
        enet_deinitialize();
        return false;
    }

    running_ = true;
    thread_ = std::thread(&NetworkClient::serviceLoop, this);
    return true;
}

void NetworkClient::disconnect()
{
    if (!running_)
    {
        return;
    }

    running_ = false;
    if (thread_.joinable())
    {
        thread_.join();
    }
    if (client_)
    {
        if (peer_)
        {
            enet_peer_disconnect(peer_, 0);
            ENetEvent event;

            while (enet_host_service(client_, &event, 3000) > 0)
            {
                if (event.type == ENET_EVENT_TYPE_DISCONNECT)
                {
                    break;
                }
            }

            enet_peer_reset(peer_);
            peer_ = nullptr;
        }
        enet_host_destroy(client_);
        client_ = nullptr;
    }
    enet_deinitialize();
}

void NetworkClient::send(const std::vector<uint8_t>& data) const
{
    if (!peer_)
    {
        return;
    }
    ENetPacket* packet = enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer_, 0, packet);
    enet_host_flush(client_);
}

void NetworkClient::setReceiveCallback(std::function<void(const std::vector<uint8_t>&)> cb)
{
    callback_ = std::move(cb);
}

void NetworkClient::serviceLoop()
{
    while (running_) {
        ENetEvent event;
        while (enet_host_service(client_, &event, 100) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE: {
                    std::vector<uint8_t> data((uint8_t*)event.packet->data, (uint8_t*)event.packet->data + event.packet->dataLength);
                    if (callback_) callback_(data);
                    enet_packet_destroy(event.packet);
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Disconnected from server\n";
                    running_ = false;
                    break;
                default:
                    break;
            }
        }
    }
}
