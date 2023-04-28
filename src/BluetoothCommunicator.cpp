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
