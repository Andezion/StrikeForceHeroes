#include "raylib.h"
#include "Game.h"
#include "NetworkServer.h"
#include <string>
#include <iostream>
#include "NetworkClient.h"
#include <chrono>

int main(int argc, char** argv)
{
    if (argc > 1)
    {
        std::string mode = argv[1];

        if (mode == "server")
        {
            uint16_t port = 1234;
            if (argc > 2) port = static_cast<uint16_t>(std::stoi(argv[2]));
            NetworkServer server(port);
            if (!server.start()) {
                std::cerr << "Failed to start server\n";
                return 1;
            }
            std::cout << "Server running on port " << port << ". Press ENTER to stop.\n";
            std::string dummy;
            std::getline(std::cin, dummy);
            server.stop();
            return 0;
        }
        if (mode == "client") {
            std::string host = "127.0.0.1";
            uint16_t port = 1234;
            if (argc > 2) host = argv[2];
            if (argc > 3) port = static_cast<uint16_t>(std::stoi(argv[3]));

            NetworkClient client;
            if (!client.connectTo(host, port)) {
                std::cerr << "Failed to connect to " << host << ":" << port << "\n";
                return 1;
            }

            client.setReceiveCallback([&](const std::vector<uint8_t>& data){
                std::string s(data.begin(), data.end());
                std::cout << "Received: " << s << "\n";
            });

            std::cout << "Connected to " << host << ":" << port << ". Type lines to send, empty line to quit." << std::endl;
            std::string line;
            while (std::getline(std::cin, line)) {
                if (line.empty()) break;
                std::vector<uint8_t> v(line.begin(), line.end());
                client.send(v);
            }

            client.disconnect();
            return 0;
        }
    }

    constexpr int screenWidth = 1800;
    constexpr int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "StrikeForceHeroes");
    HideCursor();
    SetTargetFPS(60);

    Game game(screenWidth, screenHeight);

    while (!WindowShouldClose())
    {
        const float deltaTime = GetFrameTime();

        (void)GetMousePosition();

        game.Update(deltaTime);
        game.Draw();
    }

    ShowCursor();
    CloseWindow();
    return 0;
}