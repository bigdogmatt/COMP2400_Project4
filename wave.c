#include <stdio.h>
#include <stdlib.h>
#include "wave.h"
#include <string.h>

int writeHeader( const WaveHeader* header );
int readHeader( WaveHeader* header );

int main(int argc, char **argv)
{
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
			fprintf(stderr, "Usage: wave [[-r][-s factor][-f][-o delay][-i delay][-v scale][-e delay scale] < input > output");
		}
		currentArg++;

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
