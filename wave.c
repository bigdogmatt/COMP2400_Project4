#include <stdio.h>
#include <stdlib.h>
#include "wave.h"
#include <string.h>

int writeHeader( const WaveHeader* header );
int readHeader( WaveHeader* header );

int main(int argc, char **argv)
{
	//command line input
	int currentArg = 1;
	while (currentArg < argc) {
		if (strcmp( "-r", argv[currentArg]) == 0) {
			// reverse sound
		} else if (strcmp("-s", argv[currentArg]) == 0) {
			// change speed
			currentArg++;
			double speedFactor = strtod(argv[currentArg], NULL);
		} else if (strcmp("-f", argv[currentArg]) == 0) {
			// flip channels
		} else if (strcmp("-o", argv[currentArg]) == 0) {
			// fade out
			currentArg++;
			double fadeTime = strtod(argv[currentArg], NULL);
		} else if (strcmp("-i", argv[currentArg]) == 0) {
			// fade in
			currentArg++;
			double fadeTime = strtod(argv[currentArg], NULL);
		} else if (strcmp("-v", argv[currentArg]) == 0) {
			// volume
			currentArg++;
			double volumeFactor = strtod(argv[currentArg], NULL);
		} else if (strcmp("-e", argv[currentArg]) == 0) {
			// echo
			currentArg++;
			double delay = strtod(argv[currentArg], NULL);
			currentArg++;
			double echoVolumeFactor = strtod(argv[currentArg], NULL);
		} else {
			// no such arguement
			fprintf(stderr, "Usage: wave [[-r][-s factor][-f][-o delay][-i delay][-v scale][-e delay scale] < input > output\n");
		}
		currentArg++;
	}

	/**********
	FRIDAY, MARCH 13th
	**********/

	//Wave data input
	WaveHeader header;
	readHeader(&header);

	FormatChunk fChunk = header.formatChunk;
	DataChunk dChunk = header.dataChunk;

	unsigned int numBytes = dChunk.size; //Number of bytes in the data

	unsigned short bitsPerSample = fChunk.bitsPerSample; //Used to check for error and calculate the number of samples
	if ( bitsPerSample != 16 ) {
		fprintf(stderr, "File does not have 16-bit samples\n");
	}

	unsigned short numChannels = fChunk.channels; //Used to check for error
	if ( numChannels != 2 ) {
		fprintf(stderr, "File is not stereo\n");
	}

	unsigned int sampleRate = fChunk.sampleRate; //Used to check for error
	if ( sampleRate != 44100 ) {
		fprintf(stderr, "File does not use 44,100Hz sample rate\n");
	}

	unsigned int numSamples = numBytes / (numChannels * (bitsPerSample / 8));

	short* leftChannel = (short*)malloc(numSamples * sizeof(short));
	short* rightChannel = (short*)malloc(numSamples * sizeof(short));
	if ( leftChannel == NULL || rightChannel == NULL) {
		fprintf(stderr, "Program out of memory\n");
	}

	char a = getchar();
	char b;
	short value;
	if ( a == EOF ) {
		fprintf(stderr, "Format chunk is corrupted\n");
	}
	else {
		b = getchar();
	}

	// Storing the data
	int leftIndex = 0;
	int rightIndex = 0;
	for ( int i = 0; i < numSamples; ++i ) {
		if ( b == EOF ) {
			fprintf(stderr, "File size does not match size in header\n");
		}
		else {
			value = ((short)a) << 8;
			value = value | b;
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
		fprintf(stderr, "%hu : %hu\n", leftChannel[i], rightChannel[i]);
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
