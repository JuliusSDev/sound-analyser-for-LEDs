#include "SoundAnalyser.hpp"

SoundAnalyser::SoundAnalyser(){
    err = paNoError;

    recordedSamples = new SAMPLE[FRAMES_PER_BUFFER]();  /* From now on, recordedSamples is initialised. */

    err = Pa_Initialize();
    checkIfErrorOccured();

    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        throw PortAudioException(DEVICE_NOT_FOUND_ERROR);
    }

    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = SUGGESTED_LATTENCY;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    /* Record some audio. -------------------------------------------- */
    err = Pa_OpenStream(&stream,
                        &inputParameters,
                        NULL,                  /* &outputParameters, */
                        SAMPLE_RATE,
                        FRAMES_PER_BUFFER,
                        paClipOff,      /* we won't output out of range samples so don't bother clipping them */
                        recordCallback,
                        recordedSamples);
    checkIfErrorOccured();

    err = Pa_StartStream( stream );
    checkIfErrorOccured();

    printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);
}

SoundAnalyser::~SoundAnalyser(){
    err = Pa_CloseStream(stream);
    Pa_Terminate();
    if(recordedSamples){       /* Sure it is NULL or valid. */
        delete[] recordedSamples;
    }
    if( err != paNoError ){
        fprintf( stderr, "An error occurred while using the portaudio stream\n" );
        fprintf( stderr, "Error number: %d\n", err );
        fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    }
}

void SoundAnalyser::analizeSamples(const SAMPLE *samplesBuffer){
    static float max_of_maxes = 0;
    float max = 0, val = 0;
    for(int i = 0; i < FRAMES_PER_BUFFER; i++){
        val = samplesBuffer[i];
        if( val < 0 ) val = -val; /* ABS */
        if( val > max ){
            max = val;
        }
    }

    if(max > max_of_maxes){
        max_of_maxes = max;
    }

    printf("sample max amplitude = " PRINTF_S_FORMAT ",   absolute max = " PRINTF_S_FORMAT "\n", max, max_of_maxes);
    fflush(stdout);
}

void SoundAnalyser::checkIfErrorOccured(){
    if(err != paNoError){
        throw PortAudioException(err);
    }
}

int SoundAnalyser::recordCallback( const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData ){
    const SAMPLE *samplesBuffer = (const SAMPLE*)inputBuffer;
    int finished;

    (void) outputBuffer; /* Prevent unused variable warnings. */
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;

    analizeSamples(samplesBuffer);

    //finished = paComplete;  // to stop reccording
    finished = paContinue;  // to continue reccording
    return finished;
}
