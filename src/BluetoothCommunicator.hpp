#include <iostream>
#include <unistd.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

//Commands codes
#define SIZE_OF_COMCODE 1
#define SIZE_OF_ARRAY_LENGTH 2
#define COMCODE_WAIT 0x00 //format: COMCODE    - set all LEDS to LIGHT_BRIGHTNESS
#define COMCODE_LEDS_ARRAY 0x01  //format: COMCODE, number_of_leds(2bytes), [0..LEDs](color for each led(3bytes each))     - fill LED tape by colors array
#define COMCODE_SOLID_COLOR 0x02  //format: COMCODE, color(r/g/b(3bytes))    -fill whole tape by solid color
#define COMCODE_RGB_EQ_SOUND 0x03  //format: COMCODE, color(r/g/b(3bytes))    -separate tape into low(r)/mid(g)/high(b) freq zones and fill them by colors
#define COMCODE_ONE_COLOR_EQ_SOUND 0x04  //format: COMCODE, color(r/g/b(3bytes))    -fill whole tape by 1 color which is not zero(e.g. FF0000).(if more then 1 color entered(e.g.FF00FF) -> will be chosen first)


//LEDs
#define NUMBER_OF_LEDS 50
#define NUMBER_OF_COLORS 3  // R/G/B
#define MAX_BRIGHTNESS 255

//sound light defines
#define BRIGHTNESS_THRESHOLD 150
#define DIFFERENT_MIN_MAX_FOR_DARKNESS 3
// #define NO_FULL_DARKNESS

class BluetoothCommunicator{
    public:
        int status;

        BluetoothCommunicator(std::string btMACaddress);
        ~BluetoothCommunicator();

        void sendData(uint8_t * buffer, size_t bufferSize);
        void send_RGB_EQ_SOUND(uint8_t red, uint8_t green, uint8_t blue);
        void send_ONE_COLOR_EQ_SOUND(uint8_t red, uint8_t green, uint8_t blue);
    
    private:
        bdaddr_t deviceMAC;  // device MAC address
        struct sockaddr_rc socketAddress;
        int socketNumber;
        uint8_t commandBuffer_EQ_SOUND[4];

        void add24bitColorToArray(uint8_t red, uint8_t green, uint8_t blue, uint8_t * buffer, int insertIndex);
};

class BluetoothException{
    public:
        std::string exceptionMessage;
        BluetoothException(std::string message): exceptionMessage(message){};
};