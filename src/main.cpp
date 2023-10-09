#include "SoundAnalyser.hpp"
#include "BluetoothCommunicator.hpp"
#include "TcpCommunicator.hpp"
#include <thread>
#include <chrono>

int main(){
    // CommunicatorI *bluetoothCommunicator = new BluetoothCommunicator("B8:D6:1A:A7:45:1E");
    CommunicatorI *tcpCommunicator = new TcpCommunicator(PORT);
    // SoundAnalyser myAnalyser(bluetoothCommunicator);
    SoundAnalyser myAnalyser(tcpCommunicator);
    while(1){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    // delete bluetoothCommunicator;
    delete tcpCommunicator;
    return 0;
}