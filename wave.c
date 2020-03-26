#include <stdio.h>
#include <stdlib.h>
#include "wave.h"
#include <string.h>
#include <limits.h>

int writeHeader( const WaveHeader* header );
int readHeader( WaveHeader* header );
unsigned int numSamplesCalc( WaveHeader* header );
short clampShort( double n );
void echo(short *channel[], WaveHeader *header, double delay, double volume);
void changeVolume(short channel[], WaveHeader *header, double factor);
void fadeIn(short channel[], WaveHeader *header, double seconds);
void fadeOut(short channel[], WaveHeader *header, double seconds);


int main(int argc, char **argv)
{
	//Wave data input
	WaveHeader header;
	readHeader(&header);

	unsigned int numBytes = header.dataChunk.size; //Number of bytes in the data

	unsigned short bitsPerSample = header.formatChunk.bitsPerSample; //Used to check for error and calculate the number of samples
	if ( bitsPerSample != 16 ) {
		fprintf(stderr, "File does not have 16-bit samples\n");
		return 8;
	}

	unsigned short numChannels = header.formatChunk.channels; //Used to check for error
	if ( numChannels != 2 ) {
		fprintf(stderr, "File is not stereo\n");
		return 6;
	}

	unsigned int sampleRate = header.formatChunk.sampleRate; //Used to check for error
	if ( sampleRate != 44100 ) {
		fprintf(stderr, "File does not use 44,100Hz sample rate\n");
		return 7;
	}

	unsigned int numSamples = numSamplesCalc(&header);

	short* leftChannel = (short*)malloc(numSamples * sizeof(short));
	short* rightChannel = (short*)malloc(numSamples * sizeof(short));
	if ( leftChannel == NULL || rightChannel == NULL) {
		fprintf(stderr, "Program out of memory\n");
		return 2;
	}

	int a = getchar();
	int b;
	short value;
	if ( a == EOF ) {
		fprintf(stderr, "Format chunk is corrupted\n");
		return 5;
	}
	else {
		b = getchar();
	}

	// Storing the data
	int leftIndex = 0;
	int rightIndex = 0;
	for ( int i = 0; i < numSamples*2; ++i ) {
		if ( b == EOF ) {
			fprintf(stderr, "File size does not match size in header\n");
			return 9;
		}
		else {
			value = b << 8;
			value = value | a;
			if ( (i % 2) == 0) {
				leftChannel[leftIndex] = value;
				++leftIndex;
			}
			else {
				rightChannel[rightIndex] = value;
				++rightIndex;
			}
		}
		a = getchar();
		b = getchar();
	}

	//Wave input testing
	for ( int i = 0; i < numSamples; ++i) {
		fprintf(stderr, "%hi : %hi\n", leftChannel[i], rightChannel[i]);
	}

	//command line input
	int currentArg = 1;
	while (currentArg < argc) {
		if (strcmp( "-r", argv[currentArg]) == 0) {
			// reverse sound
		} else if (strcmp("-s", argv[currentArg]) == 0) {
			// change speed
			currentArg++;
			double speedFactor = atof(argv[currentArg]);
			if ( speedFactor <= 0.0 ) {
				fprintf(stderr, "A positive number must be supplied for the speed to change");
				return 10;
			}
		} else if (strcmp("-f", argv[currentArg]) == 0) {
			// flip channels
		} else if (strcmp("-o", argv[currentArg]) == 0) {
			// fade out
			currentArg++;
			double fadeTime = atof(argv[currentArg]);
			if ( fadeTime <= 0.0 ) {
				fprintf(stderr, "A positive number must be supplied for the fade in and fade out time");
				return 11;
			}
		} else if (strcmp("-i", argv[currentArg]) == 0) {
			// fade in
			currentArg++;
			double fadeTime = atof(argv[currentArg]);
			if ( fadeTime <= 0.0) {
				fprintf(stderr, "A positive number must be supplied for the fade in and fade out time");
				return 11;
			}
		} else if (strcmp("-v", argv[currentArg]) == 0) {
			// volume
			currentArg++;
			double volumeFactor = atof(argv[currentArg]);
			if ( volumeFactor <= 0.0 ) {
				fprintf(stderr, "A positive number must be supplied for the volume to scale");
				return 12;
			}
			changeVolume(leftChannel, &header, volumeFactor);
			changeVolume(rightChannel, &header, volumeFactor);
		} else if (strcmp("-e", argv[currentArg]) == 0) {
			// echo
			currentArg++;
			double delay = atof(argv[currentArg]);
			if ( delay <= 0.0 ) {
				fprintf(stderr, "Positive number must be supplied for the echo delay and scale parameters");
				return 13;
			}
			currentArg++;
			double echoVolumeFactor = atof(argv[currentArg]);
			if ( echoVolumeFactor <= 0.0 ) {
				fprintf(stderr, "Positive number must be supplied for the echo delay and scale parameters");
				return 13;
			}
			echo(&leftChannel, &header, delay, echoVolumeFactor);
			echo(&rightChannel, &header, delay, echoVolumeFactor);
		} else {
			// no such arguement
			fprintf(stderr, "Usage: wave [[-r][-s factor][-f][-o delay][-i delay][-v scale][-e delay scale] < input > output\n");
			return 1;
		}
		currentArg++;
	}

	writeHeader(&header);

	numSamples = numSamplesCalc(&header);

	for( int i = 0; i < numSamples; ++i ) {
		putchar(leftChannel[i] & 0xFF);
		putchar(leftChannel[i] >> 8);
		putchar(rightChannel[i] & 0xFF);
		putchar(rightChannel[i] >> 8);
	}

	return 0;
}

int writeHeader( const WaveHeader* header )
{
	if( fwrite( header, sizeof( WaveHeader ), 1, stdout) != 1 )
		return 0;

	return 1;
}

int readHeader( WaveHeader* header )
{
	if( fread( header, sizeof( WaveHeader ), 1, stdin) != 1 )
		return 0;

	return 1;
}

unsigned int numSamplesCalc( WaveHeader* header )
{
	unsigned int numBytes = header->dataChunk.size;
	unsigned short numChannels = header->formatChunk.channels;
	unsigned short bitsPerSample = header->formatChunk.bitsPerSample;

	unsigned int numSamples = numBytes / (numChannels * (bitsPerSample / 8));
	return numSamples;
}

void echo(short** channel, WaveHeader *header, double delay, double volume)
{
	// Number of samples in one channel
	unsigned int samples = numSamplesCalc(header);

	// Echo offset
	unsigned int delayInSamples = header->formatChunk.sampleRate * delay;

	// The given waveform with echo added and space added for the echo
	short *newWave = (short*) malloc(
			sizeof(short) * (samples + delayInSamples));

	// Fill newWave with old sound data
	for (unsigned int i = 0; i < samples; ++i)
		newWave[i] = (*channel)[i];

	// Zero out extra space
	for (unsigned int i = samples; i < samples + delayInSamples; ++i)
		newWave[i] = 0;

	// Add in echo
	for (unsigned int i = delayInSamples; i < samples+delayInSamples; ++i) {
		// Calculate the new sound level and clamp to [MIN SHORT, MAX SHORT]
		newWave[i] = clampShort((*channel)[i - delayInSamples] * volume + newWave[i]);
	}

	// Free old wave and point the wave pointer to newWave
	free(*channel);
	*channel = newWave;

	// Because the waveform is longer, we need to alter the header
	// size data to be consistent
	header->dataChunk.size += header->formatChunk.channels
		* header->formatChunk.bitsPerSample/8 * delayInSamples;
	header->size += header->formatChunk.channels *
		header->formatChunk.bitsPerSample/8 * delayInSamples;

	return;
}

void changeVolume(short channel[], WaveHeader *header, double factor)
{
	// Number of samples in one channel
	unsigned int samples = numSamplesCalc(header);

	for (unsigned int i = 0; i < samples; ++i) {
		// Calculate the new sound level and clamp to [MIN SHORT, MAX SHORT]
		channel[i] = clampShort(channel[i] * factor);
	}
}

short clampShort(double n)
{
	if (n > SHRT_MAX)
		return SHRT_MAX;
	else if (n < SHRT_MIN)
		return SHRT_MIN;
	else
		return n;
}

unsigned int computSamples(WaveHeader *header)
{
	return header->dataChunk.size /
		(header->formatChunk.channels*header->formatChunk.bitsPerSample/8);
}


