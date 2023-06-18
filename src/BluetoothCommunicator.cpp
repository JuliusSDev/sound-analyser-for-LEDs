#include "BluetoothCommunicator.hpp"

BluetoothCommunicator::BluetoothCommunicator(std::string btMACaddress){
    str2ba(btMACaddress.c_str(), &deviceMAC);
    socketAddress.rc_bdaddr = deviceMAC;
    socketAddress.rc_family = AF_BLUETOOTH;
    socketAddress.rc_channel = (uint8_t) 1;

    socketNumber = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    status = connect(socketNumber, (struct sockaddr *)&socketAddress, sizeof(socketAddress));
    if(status == 0) {
        std::cout << "Bluetooth connection with " << btMACaddress << " established!" << std::endl;
    }else {
        perror("Connection failed!");
        throw BluetoothException("Failed to connect");
    }
    
}

BluetoothCommunicator::~BluetoothCommunicator(){
    close(socketNumber);
}

void BluetoothCommunicator::sendData(uint8_t *buffer, size_t bufferSize){
    status = write(socketNumber, buffer, bufferSize);
    if(status < 0) {
        perror("Error sending data!");
        throw BluetoothException("Failed to send data");
    }
}

void BluetoothCommunicator::add24bitColorToArray(uint8_t red, uint8_t green, uint8_t blue, uint8_t *buffer, int insertIndex){
    buffer[insertIndex] = red;
    buffer[insertIndex + 1] = green;
    buffer[insertIndex + 2] = blue;
}

void BluetoothCommunicator::send_RGB_EQ_SOUND(uint8_t red, uint8_t green, uint8_t blue){
    commandBuffer_EQ_SOUND[0] = COMCODE_RGB_EQ_SOUND;
    add24bitColorToArray(red, green, blue, commandBuffer_EQ_SOUND, SIZE_OF_COMCODE);
    sendData(commandBuffer_EQ_SOUND, SIZE_OF_COMCODE + NUMBER_OF_COLORS);
}

void BluetoothCommunicator::send_ONE_COLOR_EQ_SOUND(uint8_t red, uint8_t green, uint8_t blue){
    commandBuffer_EQ_SOUND[0] = COMCODE_ONE_COLOR_EQ_SOUND;
    add24bitColorToArray(red, green, blue, commandBuffer_EQ_SOUND, SIZE_OF_COMCODE);
    sendData(commandBuffer_EQ_SOUND, SIZE_OF_COMCODE + NUMBER_OF_COLORS);
}
