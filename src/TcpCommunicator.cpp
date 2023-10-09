#include "TcpCommunicator.hpp"

TcpCommunicator::TcpCommunicator(int port){
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket creation error!");
        throw TcpSocketException("Socket creation error");
    }
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
  
    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, LOCALHOST_ADDR, &serv_addr.sin_addr) <= 0){
        perror("Invalid IP address/ Address not supported!");
        throw TcpSocketException("Invalid IP address/ Address not supported");
    }
  
    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0){
        perror("Connection Failed!");
        throw TcpSocketException("Connection Failed");
    }
    std::cout << "Connection with " << LOCALHOST_ADDR << ":" << port << " was established" << std::endl;
}

TcpCommunicator::~TcpCommunicator(){
    close(client_fd);
}

void TcpCommunicator::sendData(uint8_t *buffer, size_t bufferSize){
    status = send(client_fd, buffer, bufferSize, 0);  //Can generate SIGPIPE signal in case of dropped connection
    if(status < 0){
        perror("Error sending data!");
        throw TcpSocketException("Error sending data");
    }
}

void TcpCommunicator::send_RGB_EQ_SOUND(uint8_t red, uint8_t green, uint8_t blue){
    std::string messageRed = makeMessageString(COLOR_CHANNEL_R, red, 0, 0);
    std::string messageGreen = makeMessageString(COLOR_CHANNEL_G, 0, green, 0);
    std::string messageBlue = makeMessageString(COLOR_CHANNEL_B, 0, 0, blue);
    sendData((uint8_t*)messageRed.c_str(), messageRed.length());
    sendData((uint8_t*)messageGreen.c_str(), messageGreen.length());
    sendData((uint8_t*)messageBlue.c_str(), messageBlue.length());
}

void TcpCommunicator::send_ONE_COLOR_EQ_SOUND(uint8_t red, uint8_t green, uint8_t blue){
    std::string message = makeMessageString(COLOR_CHANNEL_R, red, green, blue);
    sendData((uint8_t*)message.c_str(), message.length());
}

std::string TcpCommunicator::makeMessageString(uint8_t channel, uint8_t red, uint8_t green, uint8_t blue){
    return std::to_string(channel) +
            " " + std::to_string(red) +
            " " + std::to_string(green) +
            " " + std::to_string(blue) + "\n";
}
