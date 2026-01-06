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
    std::cout << "[NetworkServer] Starting...\n";
    std::cout.flush();
    
    if (running_)
    {
        std::cout << "[NetworkServer] Already running\n";
        std::cout.flush();
        return true;
    }

    std::cout << "[NetworkServer] Initializing ENet...\n";
    std::cout.flush();
    
    if (enet_initialize() != 0)
    {
        std::cerr << "[NetworkServer] ERROR: ENet initialization failed\n";
        std::cerr.flush();
        return false;
    }

    std::cout << "[NetworkServer] ENet initialized successfully\n";
    std::cout.flush();

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port_;

    std::cout << "[NetworkServer] Creating host on port " << port_ << "...\n";
    std::cout.flush();

    host_ = enet_host_create(&address, 32, 2, 0, 0);

    if (!host_)
    {
        std::cerr << "[NetworkServer] ERROR: Failed to create ENet server host (port " << port_ << " may be in use)\n";
        std::cerr.flush();
        enet_deinitialize();

        return false;
    }

    std::cout << "[NetworkServer] Host created successfully\n";
    std::cout.flush();

    running_ = true;
    thread_ = std::thread(&NetworkServer::serviceLoop, this);
    
    std::cout << "[NetworkServer] Service thread started\n";
    std::cout.flush();
    
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
