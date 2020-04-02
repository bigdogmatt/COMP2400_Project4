#ifndef WAVE_H
#define WAVE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct _FormatChunk
{
	unsigned char	ID[4];
	unsigned int	size;
	unsigned short	compression;
	unsigned short	channels;
	unsigned int	sampleRate;
	unsigned int	byteRate;
	unsigned short	blockAlign;
	unsigned short	bitsPerSample;
} FormatChunk;

typedef struct _DataChunk
{
	unsigned char	ID[4];
	unsigned int	size;
} DataChunk;

typedef struct _WaveHeader
{
	unsigned char	ID[4];
	unsigned int	size;
	unsigned char	format[4];
	FormatChunk		formatChunk;
	DataChunk		dataChunk;
} WaveHeader;

/* Helper functions */
int writeHeader( const WaveHeader* header );
int readHeader( WaveHeader* header );
unsigned int numSamplesCalc( WaveHeader* header );
short clampShort( double n );
int printError(int e);

/* Waveform functions */
int echo(short **channel, WaveHeader *header, double delay, double volume);
void changeVolume(short channel[], WaveHeader *header, double factor);
void fadeIn(short channel[], WaveHeader *header, double seconds);
void fadeOut(short channel[], WaveHeader *header, double seconds);
void reverse(short channel[], WaveHeader *header);
int changeSpeed(short** channel, WaveHeader *header, double factor);
#endif
