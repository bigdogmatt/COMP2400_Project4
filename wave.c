#include "wave.h"

enum errors { NO_ERROR, INVALID_ARG, OUT_OF_MEM, NOT_A_RIFF,
			  BAD_FORMAT_CHUNK, BAD_DATA_CHUNK, NOT_STEREO, INVALID_SAMPLE_RATE,
			  INVALID_SAMPLE_SIZE, INVALID_FILE_SIZE, INVALID_SPEED, INVALID_TIME,
			  INVALID_VOLUME, INVALID_ECHO};

const char *errorMessages[] = {
		"",
		"Usage: wave [[-r][-s factor][-f][-o delay][-i delay][-v scale][-e delay scale]"
		" < input > output",
        "Program out of memory",
        "File is not a RIFF file",
        "Format chunk is corrupted",
        "Format chunk is corrupted",
        "File is not stereo",
        "File does not use 44,100Hz sample rate",
        "File does not have 16-bit samples",
        "File size does not match size in header",
        "A positive number must be supplied for the speed change",
        "A positive number must be supplied for the fade in and fade out time",
        "A positive number must be supplied for the volume scale",
        "Positive numbers must be supplied for the echo delay and scale parameters"
};

int main(int argc, char **argv)
{
	// Wave data input
	WaveHeader header;
	readHeader(&header);

	//Used to check for error
	if( strncmp(header.ID, "RIFF", 4) )
		return printError(NOT_A_RIFF);

	//Used to check for error
	if( (strncmp(header.formatChunk.ID, "fmt ", 4)) || header.formatChunk.size != 16 || header.formatChunk.compression != 1)
		return printError(BAD_FORMAT_CHUNK);

	//Used to check for error
	if( strncmp(header.dataChunk.ID, "data", 4) )
		return printError(BAD_DATA_CHUNK);

	//Used to check for error
	if ( header.formatChunk.channels != 2 )
		return printError(NOT_STEREO);

	//Used to check for error
	if ( header.formatChunk.sampleRate != 44100 )
		return printError(INVALID_SAMPLE_RATE);

	//Used to check for error
	if ( header.formatChunk.bitsPerSample != 16 )
		return printError(INVALID_SAMPLE_SIZE);

	//Get the number of samples needed to allocate memory for the channels
	unsigned int numSamples = numSamplesCalc(&header);

	short* leftChannel = (short*)malloc(numSamples * sizeof(short));
	short* rightChannel = (short*)malloc(numSamples * sizeof(short));

	//Used to check for error
	if ( leftChannel == NULL || rightChannel == NULL)
		return printError(OUT_OF_MEM);

	//Get the characters of the frist sample
	int a = getchar();
	int b = getchar();
	short value;

	int leftIndex = 0;
	int rightIndex = 0;

	//Loop up to 2 times the number of samples because we need to store
	//the number of samples for each channel
	for ( int i = 0; i < numSamples*2; ++i ) {
		//Used to check for error if we hit the EOF before expected
		if ( b == EOF )
			return printError(INVALID_FILE_SIZE);
		else {
			//Combine the character values into a short
			value = b << 8;
			value = value | a;

			//Store in the left channel on even iterations and the right channel
			//on odd iterations
			if ( (i % 2) == 0) {
				leftChannel[leftIndex] = value;
				++leftIndex;
			}
			else {
				rightChannel[rightIndex] = value;
				++rightIndex;
			}
		}
		//Get the characters for the next sample
		a = getchar();
		b = getchar();
	}


	//command line input
	int currentArg = 1;
	while (currentArg < argc) {
		if (strcmp( "-r", argv[currentArg]) == 0) {
			// Reverse sound
			reverse(leftChannel, &header);
			reverse(rightChannel, &header);
		} else if (strcmp("-s", argv[currentArg]) == 0) {
			// Change speed
			currentArg++;
			// Convert the string factor into a double
			double speedFactor = atof(argv[currentArg]);
			// Used to check for error
			if ( speedFactor <= 0.0 )
				return printError(INVALID_SPEED);
			// Change the speed of the right and left channels
			// changeSpeed returns an int equal to the bytes added for the
			// change in the channel sizes used to update the header
			int bytesAdded = changeSpeed(&leftChannel, &header, speedFactor);
			bytesAdded = changeSpeed(&rightChannel, &header, speedFactor);
			header.dataChunk.size += bytesAdded;
			header.size += bytesAdded;

		} else if (strcmp("-f", argv[currentArg]) == 0) {
			// Flip channels
			short *temp = leftChannel;
			leftChannel = rightChannel;
			rightChannel = temp;
		} else if (strcmp("-o", argv[currentArg]) == 0) {
			// Fade out
			currentArg++;
			double fadeTime = atof(argv[currentArg]);
			//Used to check for error
			if ( fadeTime <= 0.0 )
				return printError(INVALID_TIME);

			fadeOut(leftChannel, &header, fadeTime);
			fadeOut(rightChannel, &header, fadeTime);
		} else if (strcmp("-i", argv[currentArg]) == 0) {
			// Fade in
			currentArg++;
			double fadeTime = atof(argv[currentArg]);
			//Used to check for error
			if ( fadeTime <= 0.0)
				return printError(INVALID_TIME);

			fadeIn(leftChannel, &header, fadeTime);
			fadeIn(rightChannel, &header, fadeTime);
		} else if (strcmp("-v", argv[currentArg]) == 0) {
			// Change volume
			currentArg++;
			double volumeFactor = atof(argv[currentArg]);
			//Used to check for error
			if ( volumeFactor <= 0.0 )
				return printError(INVALID_VOLUME);

			changeVolume(leftChannel, &header, volumeFactor);
			changeVolume(rightChannel, &header, volumeFactor);
		} else if (strcmp("-e", argv[currentArg]) == 0) {
			// Echo
			currentArg++;
			double delay = atof(argv[currentArg]);
			//Used to check for error
			if ( delay <= 0.0 )
				return printError(INVALID_ECHO);

			currentArg++;
			double echoVolumeFactor = atof(argv[currentArg]);
			//Used to check for error
			if ( echoVolumeFactor <= 0.0 )
				return printError(INVALID_ECHO);

			// echo returns an int equal to the bytes added for the
			// change in the channel sizes used to update the header
			int bytesAdded = echo(&leftChannel, &header, delay, echoVolumeFactor);
			echo(&rightChannel, &header, delay, echoVolumeFactor);
			header.dataChunk.size += bytesAdded;
			header.size += bytesAdded;
		} else
			return printError(INVALID_ARG);
		currentArg++;
	}

	/* Write data into stdout */
	writeHeader(&header);

	//Get the new sample size after the .wav file is changed
	numSamples = numSamplesCalc(&header);

	//Undo the characters to short converstion that was done when storing data
	for( int i = 0; i < numSamples; ++i ) {
		putchar(leftChannel[i] & 0xFF);
		putchar(leftChannel[i] >> 8);
		putchar(rightChannel[i] & 0xFF);
		putchar(rightChannel[i] >> 8);
	}

	//Free the allocated memory
	free(leftChannel);
	free(rightChannel);
	return NO_ERROR;
}

/* Print the corresponding error message to standard error and return e */
int printError(int e)
{
	fprintf(stderr, "%s\n", errorMessages[e]);
	return e;
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

/* returns the number of sampels in one channel */
unsigned int numSamplesCalc( WaveHeader* header )
{
	unsigned int numSamples = header->dataChunk.size / (header->formatChunk.channels *
		(header->formatChunk.bitsPerSample / 8));
	return numSamples;
}

/* Adds an echo to the channel and returns the time offset in bytes */
int echo(short** channel, WaveHeader *header, double delay, double volume)
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

	// Free old wave and point it to newWave
	free(*channel);
	*channel = newWave;

	// Return the size difference in bytes
	return header->formatChunk.channels	* header->formatChunk.bitsPerSample/8
		* delayInSamples;
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

/* Returns a short of double with no overflow */
short clampShort(double n)
{
	if (n > SHRT_MAX)
		return SHRT_MAX;
	else if (n < SHRT_MIN)
		return SHRT_MIN;
	else
		return n;
}

void fadeIn(short channel[], WaveHeader *header, double seconds)
{
	// Number of samples in one channel
	unsigned int samples = numSamplesCalc(header);
	unsigned int fadeCount = header->formatChunk.sampleRate * seconds;

	for (unsigned int i = 0; i < fadeCount && i < samples; ++i) {
		double fadeFactor = (i/ (double)fadeCount) * (i/ (double)fadeCount);
		channel[i] *= fadeFactor;
	}

	return;
}

void fadeOut(short channel[], WaveHeader *header, double seconds)
{
	// Number of samples in one channel
	unsigned int samples = numSamplesCalc(header);
	unsigned int fadeCount = header->formatChunk.sampleRate * seconds;

	for (unsigned int i = samples - fadeCount; i < samples; ++i) {
		double x = i - (samples - fadeCount);
		double fadeFactor = (1 - x/fadeCount) * (1 - x/fadeCount);
		channel[i] *= fadeFactor;
	}

	return;
}

void reverse(short channel[], WaveHeader *header)
{
	unsigned int samples = numSamplesCalc(header);
	for (int i = 0; i < samples/2; ++i) {
		short temp = channel[i];
		channel[i] = channel[samples-i];
		channel[samples-i] = temp;
	}
}

/* Change the speed of channel and return the size difference in bytes.
 * A factor of 2 corresponds to the new channel being twice as fast. */
int changeSpeed(short **channel, WaveHeader *header, double factor)
{
	unsigned int samples = numSamplesCalc(header);

	// Create array to hold the modified channel
	unsigned int newSize = samples / factor;
	short *newChannel = (short*) malloc(newSize * sizeof(short));

	// Fill new array
	for (int i = 0; i < newSize; ++i)
		newChannel[i] = (*channel)[(int) (i * factor)];

	// change channel pointer
	free(*channel);
	*channel = newChannel;

	return header->formatChunk.channels	* header->formatChunk.bitsPerSample/8
		* (newSize - samples);
}
