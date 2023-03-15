#include "SoundAnalyser.hpp"

SoundAnalyser::SoundAnalyser():dbfsRefference(0){
    inAnaliseBuffer = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FRAMES_PER_BUFFER);
    outAnaliseBuffer = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FRAMES_PER_BUFFER);
    ftransPlan = fftw_plan_dft_1d(FRAMES_PER_BUFFER, inAnaliseBuffer, outAnaliseBuffer, FFTW_FORWARD, FFTW_MEASURE); // probably fftw_backward
    fqDBFS_linearSpectrum = new double[FREQ_MAGNITUDES_BUFFER_SIZE];

    err = paNoError;
    err = Pa_Initialize();
    checkIfErrorOccured();

    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    if (inputParameters.device == paNoDevice) {
        std::cerr << "Error: No default input device." << std::endl;
        throw PortAudioException(DEVICE_NOT_FOUND_ERROR);
    }

    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = SUGGESTED_LATTENCY;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    analyzerPtr = this;

    /* Record some audio. -------------------------------------------- */
    err = Pa_OpenStream(&stream,
                        &inputParameters,
                        NULL,                  /* &outputParameters, */
                        SAMPLE_RATE,
                        FRAMES_PER_BUFFER,
                        paClipOff,      /* we won't output out of range samples so don't bother clipping them */
                        recordCallback,
                        analyzerPtr);
    checkIfErrorOccured();

    err = Pa_StartStream( stream );
    checkIfErrorOccured();


    std::cout << std::endl << "=== Now recording!! Please speak into the microphone. ===" << std::endl;
}

SoundAnalyser::~SoundAnalyser(){
    err = Pa_CloseStream(stream);
    Pa_Terminate();
    if( err != paNoError ){
        std::cerr << "An error occurred while using the portaudio stream" << std::endl;
        std::cerr << "Error number: " << err << std::endl;
        std::cerr << "Error message: " << Pa_GetErrorText(err) << std::endl;
    }

    fftw_destroy_plan(ftransPlan);
    fftw_free(inAnaliseBuffer); fftw_free(outAnaliseBuffer);
    delete[] fqDBFS_linearSpectrum;
}

void SoundAnalyser::analizeSamples(const SAMPLE *samplesBuffer){
    for(int i = 0; i < FRAMES_PER_BUFFER; i++){
        inAnaliseBuffer[i][0] = static_cast<double>(samplesBuffer[i]);
        inAnaliseBuffer[i][1] = 0;
    }

    fftw_execute(ftransPlan);

    //TODO move part in this function bellow to another thread?
    
    double max_mag_db = -DBL_MAX, max_mag_db_index = 0, sampleMagnitude = 0, relativeMagnitude;
    for(int i = 0; i < FREQ_MAGNITUDES_BUFFER_SIZE; i++){
        sampleMagnitude = calcMagnitude(outAnaliseBuffer[i]);
        relativeMagnitude = sampleMagnitude / dbfsRefference;
        if(relativeMagnitude != 0){
            fqDBFS_linearSpectrum[i] = 20 * std::log10(sampleMagnitude / dbfsRefference);
        }else{
            fqDBFS_linearSpectrum[i] = SILENT_DBFS;
        }
        if(max_mag_db < fqDBFS_linearSpectrum[i]){
            max_mag_db = fqDBFS_linearSpectrum[i];
            max_mag_db_index = i;
        }
        if(sampleMagnitude > dbfsRefference){
            dbfsRefference = sampleMagnitude;
        }
    }

    printf("Max_mag_db = %f, at freq = %5.2f,   div = %f         ref = %f\n", 
            max_mag_db, max_mag_db_index * ((SAMPLE_RATE/2)/FREQ_MAGNITUDES_BUFFER_SIZE), std::log10(sampleMagnitude / dbfsRefference), dbfsRefference);
}

double SoundAnalyser::calcMagnitude(const fftw_complex &realSample){
    return std::sqrt(realSample[0]*realSample[0] + realSample[1]*realSample[1]);
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
    SoundAnalyser* analizerPtr = (SoundAnalyser*) userData;
    int finished;

    (void) outputBuffer; /* Prevent unused variable warnings. */
    (void) timeInfo;
    (void) statusFlags;

    analizerPtr->analizeSamples(samplesBuffer);

    //finished = paComplete;  // to stop reccording
    finished = paContinue;  // to continue reccording
    return finished;
}