#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define HEADER_SIZE 22

void print_usage() {
    fprintf(stderr, "Usage: addecho [-d delay] [-v volume_scale]  <sourcewave> <destwave>\n");
} 

int main(int argc, char *argv[]) {
     /* Default Values */ 
     short volume = 4;
     unsigned  short delay = 8000;

    /* Argument length check */    
     if (argc <  3 || argc > 7) {
         print_usage();
         exit(1);
    }
   
    int option;
    int sourceFlag = 0; 
    int destFlag = 0;
    int count = 1;
    char *end;
    while (count < argc) {
         if ((option = getopt(argc, argv,"d:v:")) != -1) {
             // Optional argument checks
             switch (option) {
                 case 'd': { //Delay argument
                    delay = strtol(optarg, &end, 10);
		    count += 2;
                    if (delay <= 0) {
                         printf("Error: delay must be positive.");
                         exit(1);
                     }
                    break;
                 }
                 case 'v': { //Volume argument
                    volume  = strtol(optarg, &end, 10);
		    count += 2;
                    if (volume <= 0) {
                         printf("Error: volume must be positive.");
                         exit(1);
                     }
                    break;
                 }
		 case '?': { //Exit with invalid argument
		     exit(1); 
		 }
                 default:
                     break;
            }
        }
        else { //Mandatory arguments
             if(count+2 == argc) {
                 sourceFlag = 1;
		 count++;
             } 
             if(count+1 == argc) {
                 destFlag = 1;
		 count++;
             }
       }
  }
    /* Test if one of the .wav files were not given */
    if (sourceFlag == 0 || destFlag == 0) {
         print_usage();
         exit(1);
    }

    /* Open files from cmd line arguments */
    FILE *input = fopen(argv[argc-2], "rb");
    if (input ==  NULL) {
          perror("fopen");
          exit(1);
    } 
    FILE *output = fopen(argv[argc-1], "wb");
    
    short header[HEADER_SIZE];
    /* Read header into buffer */
    int readHeader = fread(header, sizeof(short), HEADER_SIZE, input);
    if (readHeader == 0) { //Error or empty file.
        if (ferror(input) != 0) {
          printf("Error: Incorrect/empty file\n");
          exit(1);
        }
    }
    /* Increase bytes 2 and 20 by delay*2 */ 
    unsigned  int *sizeptr;
    sizeptr = (unsigned int *)(header+2);
    *sizeptr += (delay * 2);
    sizeptr = (unsigned int *)(header+20);;   //Move to short 20
    *sizeptr += (delay * 2);
  
    /* Write header from buffer to output file. */
    if((fwrite(header, sizeof(short), HEADER_SIZE, output)) != HEADER_SIZE)
    {
        if(ferror(output)) {
            perror("fwrite");
        } 
        else {
            fprintf(stderr, "Unexpected end of file \n");
        }
          exit(1);
    }

    /* Set echo buffer to be length of delay */
    short *echo = (short *)malloc(delay * sizeof(short));
    if (echo == NULL) {
      perror("malloc");
      exit(1);
    }
    
    /* Write, mix original samples to output file */
    short orig[1];
    int numSamples  = 0;
    int echoIndex = 0;
    while((fread(orig, sizeof(short), 1, input)) > 0) {
        /* Write original sample to output */
        if (numSamples < delay) { 
            fwrite(orig, sizeof(short), 1, output);
            echo[numSamples] = (orig[0]/volume);
        } 
        /* Mix original sample after delay with echo buffer */
        else {
            if (echoIndex  == delay) {
                echoIndex  = 0; //Reset to start at beg of buffer
            } 
            short new = orig[0]; //Temp variable to store next sample
            orig[0] += echo[echoIndex]; //Mix original sample
            echo[echoIndex] = (new/volume);
            echoIndex++;
            fwrite(orig, sizeof(short), 1, output);
        }
        
        numSamples++;
    }

    /* If delay is greater than number of samples, write 0's */
    short emptySamples = 0;
    int x = delay - numSamples;
    if (x > 0) {
       int k;
       for (k=0; k < x; k++)
           fwrite(&emptySamples, sizeof(emptySamples), 1, output);
    }
  
    /* Write remaining echo buffer to file */
    if (x > 0) { //Write mixed original samples.
        if((fwrite(echo, sizeof(short), numSamples, output)) != numSamples) {
            if(ferror(output)) {
                perror("fwrite");
            } else {
               fprintf(stderr, "Unexpected end of file \n");
            }
        exit(1);
        }
    } 
    else { //Leftover  samples are in beginning of echo buffer
        int diff = delay - echoIndex;
        if((fwrite(&echo[echoIndex], sizeof(short), diff, output)) != diff) {
            if(ferror(output)) {
                perror("fwrite");
            } else {
               fprintf(stderr, "Unexpected end of file \n");
            }
        exit(1);
        } 

        if((fwrite(echo, sizeof(short), echoIndex, output)) != echoIndex ) {
            if(ferror(output)) {
                perror("fwrite");
            } else {
               fprintf(stderr, "Unexpected end of file \n");
            }
        exit(1);
        }
    }

    /* Clean up and close up */
    free(echo);
    fclose(input);
    fclose(output);
 
    return 0;
}
