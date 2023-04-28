#include <iostream>
#include <unistd.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

//Commands codes
#define SIZE_OF_COMCODE 1
#define SIZE_OF_ARRAY_LENGTH 2
#define COMCODE_WAIT 0x00
#define COMCODE_SOUND_LEDS 0x01

//LEDs
#define NUMBER_OF_LEDS 100
#define NUMBER_OF_COLORS 3  // R/G/B
#define MAX_BRIGHTNESS 255

class BluetoothCommunicator{
    public:
        int status;

        BluetoothCommunicator(std::string btMACaddress);
        ~BluetoothCommunicator();

        void sendData(uint8_t * buffer, size_t bufferSize);
    
    private:
        bdaddr_t deviceMAC;  // device MAC address
        struct sockaddr_rc socketAddress;
        int socketNumber;
};

class BluetoothException{
    public:
        std::string exceptionMessage;
        BluetoothException(std::string message): exceptionMessage(message){};
};