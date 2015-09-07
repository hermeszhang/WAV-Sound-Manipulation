# WAV-Sound-Manipulation
C Programs to remove vocals and add echo's to wav sound files.

### Remvocals
* Removes vocals from stereo sound files using an algorithm.

Command Line Usage:

remvocals sourcewav destwav 

### Addecho
* Adds echo to a mono sound file.

Command Line Usage:

addecho [-d delay] [-v volume_scale] sourcewav destwav 
* Where delay and volume_scale are integers

#### Compiling

gcc -Wall -g -o remvocals remvocals.c

gcc -Wall -g -o addecho addecho.c
