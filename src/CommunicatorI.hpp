#pragma once
#include <cstdint>
#include <cstring>

class CommunicatorI{
    public:
        int status;
        
        virtual ~CommunicatorI(){};
        virtual void sendData(uint8_t * buffer, size_t bufferSize) = 0;
        virtual void send_RGB_EQ_SOUND(uint8_t red, uint8_t green, uint8_t blue) = 0;
        virtual void send_ONE_COLOR_EQ_SOUND(uint8_t red, uint8_t green, uint8_t blue) = 0;
};