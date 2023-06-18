#include "SoundAnalyser.hpp"

SoundAnalyser::SoundAnalyser(std::string bluetoothMACaddress):
                            bluetoothCommunicator(bluetoothMACaddress), dbfsRefference(DBL_EPSILON){
    inAnaliseBuffer = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FRAMES_PER_BUFFER);
    outAnaliseBuffer = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FRAMES_PER_BUFFER);
    ftransPlan = fftw_plan_dft_1d(FRAMES_PER_BUFFER, inAnaliseBuffer, outAnaliseBuffer, FFTW_FORWARD, FFTW_MEASURE); // probably fftw_backward
    fqDBFS_linearSpectrum = new double[FREQ_MAGNITUDES_BUFFER_SIZE];
    preparedLedArray.size = NUMBER_OF_LEDS*NUMBER_OF_COLORS + SIZE_OF_ARRAY_LENGTH + SIZE_OF_COMCODE;
    preparedLedArray.array = new uint8_t[preparedLedArray.size];

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
    delete[] preparedLedArray.array;
}

void SoundAnalyser::reset_dbfsRefference(){
    dbfsRefference = DBL_EPSILON;
}

void SoundAnalyser::analizeSamples(SamplesBuffer samplesBuffer){
    for(int i = 0; i < FRAMES_PER_BUFFER; i++){
        inAnaliseBuffer[i][0] = static_cast<double>(samplesBuffer.arrPtr[i]);
        inAnaliseBuffer[i][1] = 0; //second chanel
    }

    fftw_execute(ftransPlan);

    double sampleMagnitude = 0, relativeMagnitude, frequence;
    filtersValues.clearFiltersHighestValues();  
    for(int i = 0; i < FREQ_MAGNITUDES_BUFFER_SIZE; i++){
        sampleMagnitude = calcMagnitude(outAnaliseBuffer[i]);
        if(sampleMagnitude > dbfsRefference){
            dbfsRefference = sampleMagnitude;
        }
        relativeMagnitude = sampleMagnitude / dbfsRefference;

        if(relativeMagnitude != 0){
            fqDBFS_linearSpectrum[i] = 20 * std::log10(relativeMagnitude);
        }else{
            fqDBFS_linearSpectrum[i] = SILENT_DBFS;
        }

        frequence = i * ((SAMPLE_RATE/2)/FREQ_MAGNITUDES_BUFFER_SIZE);

        if(frequence <= LOW_MIDDLE_SEPARATION_FREQUENCY){
            if(fqDBFS_linearSpectrum[i] > filtersValues.lowFilter.highestDBFS){
                filtersValues.lowFilter.highestDBFS = fqDBFS_linearSpectrum[i];
            }
        }else if(frequence <= MIDDLE_HIGH_SEPARATION_FREQUENCY){
            if(fqDBFS_linearSpectrum[i] > filtersValues.midFilter.highestDBFS){
                filtersValues.midFilter.highestDBFS = fqDBFS_linearSpectrum[i];
            }
        }else{
            if(fqDBFS_linearSpectrum[i] > filtersValues.highFilter.highestDBFS){
                filtersValues.highFilter.highestDBFS = fqDBFS_linearSpectrum[i];
            }
        }
    }
    convertRelativeMagsToBrightnessInFilters();
    prepareSoundLedArray();
    printf("  low = %f,         mid = %f,        high = %f\n", filtersValues.lowFilter.highestDBFS, 
                                        filtersValues.midFilter.highestDBFS, 
                                        filtersValues.highFilter.highestDBFS);
    printf("%f <> %f, %f <> %f, %f <> %f\n", filtersValues.lowFilter.minLimitDBFS, filtersValues.lowFilter.maxLimitDBFS,
                                            filtersValues.midFilter.minLimitDBFS, filtersValues.midFilter.maxLimitDBFS,
                                            filtersValues.highFilter.minLimitDBFS, filtersValues.highFilter.maxLimitDBFS);
    printf("brtns-low = %d,      brtns-mid = %d,     brtns-high = %d\n", filtersValues.lowFilter.ledBrightness, 
                                        filtersValues.midFilter.ledBrightness, 
                                        filtersValues.highFilter.ledBrightness);
    bluetoothCommunicator.sendData(preparedLedArray.array, preparedLedArray.size);  // TODO make a prepare data function. it will prepare the array with leds colors + command code+ length. Then send it with btComm.sendData()
}

void SoundAnalyser::convertRelativeMagsToBrightnessInFilters(){
    adjustFilterLimits(&filtersValues.lowFilter);
    adjustFilterLimits(&filtersValues.midFilter);
    adjustFilterLimits(&filtersValues.highFilter);
    filtersValues.lowFilter.ledBrightness = calcLedBrightness(filtersValues.lowFilter);
    filtersValues.midFilter.ledBrightness = calcLedBrightness(filtersValues.midFilter);
    filtersValues.highFilter.ledBrightness = calcLedBrightness(filtersValues.highFilter);
}

void SoundAnalyser::prepareSoundLedArray(){
    preparedLedArray.array[0] = COMCODE_SOUND_LEDS;
    preparedLedArray.array[1] = 0;
    preparedLedArray.array[2] = NUMBER_OF_LEDS;
    for(int i = SIZE_OF_COMCODE + SIZE_OF_ARRAY_LENGTH; 
            i < ((NUMBER_OF_LEDS * NUMBER_OF_COLORS) + SIZE_OF_COMCODE + SIZE_OF_ARRAY_LENGTH); 
            // i += NUMBER_OF_COLORS){
            i += 5*NUMBER_OF_COLORS){
        preparedLedArray.array[i] = filtersValues.lowFilter.ledBrightness; //R
        preparedLedArray.array[i+1] = 0; //G
        preparedLedArray.array[i+2] = 0; //B
        preparedLedArray.array[i + (1*NUMBER_OF_COLORS)] = 0;//R
        preparedLedArray.array[i + (1*NUMBER_OF_COLORS)+1] = filtersValues.midFilter.ledBrightness;//G
        preparedLedArray.array[i + (1*NUMBER_OF_COLORS)+2] = 0;//B
        preparedLedArray.array[i + (2*NUMBER_OF_COLORS)] = 0;//R
        preparedLedArray.array[i + (2*NUMBER_OF_COLORS)+1] = 0;//G
        preparedLedArray.array[i + (2*NUMBER_OF_COLORS)+2] = filtersValues.highFilter.ledBrightness;//B
        preparedLedArray.array[i + (3*NUMBER_OF_COLORS)] = 0;//R
        preparedLedArray.array[i + (3*NUMBER_OF_COLORS)+1] = filtersValues.midFilter.ledBrightness;//G
        preparedLedArray.array[i + (3*NUMBER_OF_COLORS)+2] = 0;//B
        preparedLedArray.array[i + (4*NUMBER_OF_COLORS)] = filtersValues.lowFilter.ledBrightness;//R
        preparedLedArray.array[i + (4*NUMBER_OF_COLORS)+1] = 0;//G
        preparedLedArray.array[i + (4*NUMBER_OF_COLORS)+2] = 0;//B
    }
}

double SoundAnalyser::calcMagnitude(const fftw_complex &realSample){
    return std::sqrt(realSample[0]*realSample[0] + realSample[1]*realSample[1]);
}

void SoundAnalyser::adjustFilterLimits(FilteredValues* filter){
    if((filter->highestDBFS > filter->minLimitDBFS) && (filter->highestDBFS < filter->maxLimitDBFS)){
        filter->minLimitDBFS += MIN_ADJUST_STEP;
        filter->maxLimitDBFS -= MAX_ADJUST_STEP;
    }
    if(filter->highestDBFS > filter->maxLimitDBFS){
        filter->maxLimitDBFS = filter->highestDBFS;
        //move minLimit closer to maxLimit(the same difference)????
    }else if(filter->highestDBFS < filter->minLimitDBFS){
        filter->minLimitDBFS = filter->highestDBFS;
        //move maxLimit closer to minLimit on minLimit - highestDBFS(the same difference)????
    }
}

uint8_t SoundAnalyser::calcLedBrightness(const FilteredValues &filter){
    uint8_t value = (uint8_t)std::round(((filter.highestDBFS - filter.minLimitDBFS)
                                /(filter.maxLimitDBFS - filter.minLimitDBFS)) * MAX_BRIGHTNESS);
    if((filter.maxLimitDBFS - filter.minLimitDBFS) <= 3){
        return 0;
    }
    if(value <= 150){
        return 0;
    }
    return value;
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

    std::thread analyzingThread(&SoundAnalyser::analizeSamples, analizerPtr, SamplesBuffer(samplesBuffer, FRAMES_PER_BUFFER));
    analyzingThread.detach();
    // analizerPtr->analizeSamples(SamplesBuffer(samplesBuffer, FRAMES_PER_BUFFER));

    //finished = paComplete;  // to stop reccording
    finished = paContinue;  // to continue reccording
    return finished;
}

SoundAnalyser::SamplesBuffer::SamplesBuffer(const SamplesBuffer & parent) : buffSize(parent.buffSize){
    arrPtr = new SAMPLE[buffSize]{0};
    for(size_t i = 0; i < buffSize; i++){
        arrPtr[i] = parent.arrPtr[i];
    }
}

SoundAnalyser::SamplesBuffer::SamplesBuffer(const SAMPLE *samplesBuffer, size_t buffSize) : buffSize(buffSize){
    arrPtr = new SAMPLE[buffSize]{0};
    for(size_t i = 0; i < buffSize; i++){
        arrPtr[i] = samplesBuffer[i];
    }
}

SoundAnalyser::SamplesBuffer::~SamplesBuffer(){
    delete[] arrPtr;
}
