#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include <fftw3.h>
#include "portaudio.h"


#define SAMPLE_RATE  (48000) // 44100 was - 48000 in system
#define FRAMES_PER_BUFFER (512) //512 was (93.75HZ analizing) - 4096 good - 1024 in system propably
#define NUM_CHANNELS    (1)
#define SUGGESTED_LATTENCY (0.01) // 0.01 seconds LATTENCY dellay (according to 100Hz refreshing)
/* #define DITHER_FLAG     (paDitherOff) */
#define DITHER_FLAG     (0) 
#define DEVICE_NOT_FOUND_ERROR (404)
#define FREQ_MAGNITUDES_BUFFER_SIZE ((FRAMES_PER_BUFFER / 2) + 1)

/* Sample format. */
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"

/*
Should not be more than 1 instance
*/
class SoundAnalyser{
    public:
        double*             fqMagnitudes;

        SoundAnalyser();
        SoundAnalyser(const SoundAnalyser &parent) = delete;
        void operator=(const SoundAnalyser &parent) = delete;
        ~SoundAnalyser();

    private:
        PaStreamParameters  inputParameters;
        PaStream*           stream;
        PaError             err;
        SoundAnalyser*      analyzerPtr;
        fftw_complex        *inAnaliseBuffer, *outAnaliseBuffer;
        fftw_plan           ftransPlan;

        void checkIfErrorOccured();
        void analizeSamples(const SAMPLE* samplesBuffer);  //TODO have to be a thread which will have while(1) loop?, at the end of the loop thread will be blocked. recordCallback will unblockthread and update information(samplesBuffer)in it

        /* This routine will be called by the PortAudio engine when audio is needed.
        ** It may be called at interrupt level on some machines so don't do anything
        ** that could mess up the system like calling malloc() or free().
        */
        static int recordCallback( const void *inputBuffer, void *outputBuffer,
                                    unsigned long framesPerBuffer,
                                    const PaStreamCallbackTimeInfo* timeInfo,
                                    PaStreamCallbackFlags statusFlags,
                                    void *userData );
};

class PortAudioException{
    public:
        int errorCode;
        PortAudioException(int errorCode):errorCode(errorCode){};
};



