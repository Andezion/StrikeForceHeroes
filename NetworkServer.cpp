#include <enet/enet.h>
#include "NetworkServer.h"
#include <iostream>

NetworkServer::NetworkServer(const uint16_t port)
    : port_(port) {}

NetworkServer::~NetworkServer()
{
    stop();
}

bool NetworkServer::start()
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

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port_;

    host_ = enet_host_create(&address, 32, 2, 0, 0);

    if (!host_)
    {
        std::cerr << "Failed to create ENet server host\n";
        enet_deinitialize();

        return false;
    }

    running_ = true;
    thread_ = std::thread(&NetworkServer::serviceLoop, this);
    return true;
}

void NetworkServer::stop()
{
    if (!running_)
    {
        return;
    }

    running_ = false;

    if (thread_.joinable()) thread_.join();
    if (host_)
    {
        enet_host_destroy(host_);
        host_ = nullptr;
    }
    enet_deinitialize();
}

void NetworkServer::broadcast(const std::vector<uint8_t>& data) const
{
    if (!host_)
    {
        return;
    }

    ENetPacket* packet = enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(host_, 0, packet);
    enet_host_flush(host_);
}

void NetworkServer::serviceLoop() const
{
    while (running_)
    {
        ENetEvent event;
        while (enet_host_service(host_, &event, 100) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    std::cout << "Client connected from " << static_cast<int>(event.peer->address.host) << ":"
                                                          << event.peer->address.port << "\n";

                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    std::cout << "Received packet of length " << event.packet->dataLength << "\n";

                    enet_host_broadcast(host_, event.channelID, event.packet);
                    enet_packet_destroy(event.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Client disconnected\n";
                    event.peer->data = nullptr;
                    break;
                default:
                    break;
            }
        }
    }
}
