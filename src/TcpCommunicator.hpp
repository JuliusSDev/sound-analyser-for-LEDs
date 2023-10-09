#pragma once
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include "CommunicatorI.hpp"

#define LOCALHOST_ADDR ("127.0.0.1")
#define PORT (14112)

#define COLOR_CHANNEL_R (0)
#define COLOR_CHANNEL_G (8)
#define COLOR_CHANNEL_B (16)

class TcpCommunicator : public CommunicatorI{
    public:
        TcpCommunicator(int port);
        ~TcpCommunicator();

        void sendData(uint8_t * buffer, size_t bufferSize) override;
        void send_RGB_EQ_SOUND(uint8_t red, uint8_t green, uint8_t blue) override;
        void send_ONE_COLOR_EQ_SOUND(uint8_t red, uint8_t green, uint8_t blue) override;

    private:
        int client_fd;
        struct sockaddr_in serv_addr;
        std::string makeMessageString(uint8_t channel, uint8_t red, uint8_t green, uint8_t blue);
};

class TcpSocketException{
    public:
        std::string exceptionMessage;
        TcpSocketException(std::string message): exceptionMessage(message){};
};