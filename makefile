wave: wave.c wave.h
	gcc -Wall -o wave wave.c

clean:
	rm -f *.o wave
