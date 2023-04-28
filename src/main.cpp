#include "SoundAnalyser.hpp"
#include <thread>
#include <chrono>

int main(){
    SoundAnalyser myAnalyser("B8:D6:1A:A7:45:1E");
    while(1){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}