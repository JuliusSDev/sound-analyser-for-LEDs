#include "SoundAnalyser.hpp"
#include <thread>
#include <chrono>

int main(){
    SoundAnalyser myAnalyser;
    while(1){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}