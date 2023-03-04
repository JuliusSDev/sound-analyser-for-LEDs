#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"

#define SAMPLE_RATE  (48000) // 44100 was - 48000 in system
#define FRAMES_PER_BUFFER (1024) //512 was (93.75HZ analizing) - 4096 good - 1024 in system propably
#define NUM_SECONDS     (5)
#define NUM_CHANNELS    (1)
#define SUGGESTED_LATTENCY (0.01) // 0.01 seconds LATTENCY dellay (according to 100Hz refreshing)
/* #define DITHER_FLAG     (paDitherOff) */
#define DITHER_FLAG     (0) 

/* Sample format. */

#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"

void analizeSamples(const SAMPLE* samplesBuffer);

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int recordCallback( const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData );