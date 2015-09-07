#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define HEADER_SIZE 44

int main(int argc, char *argv[]) {

   /* Check for three arguments  passed into the program
    * contains the name of program, source wav file,
    * output wav file.
    */
   if (argc != 3) {
      fprintf(stderr, "Usage: remvocals <sourcewav> <destwav>");
      exit(1);
   }

   /* Open sourcewav file for reading. */
   FILE *input = fopen(argv[1], "rb");
   if (input ==  NULL) {
      perror("fopen");
      exit(1);
   }

   FILE *output = fopen(argv[2], "wb");
 
   /* Get size of file to allocate mem for buffer
    * Create Short array, to be used to store samples.
    */
   off_t  size;
   fseek(input, 0, SEEK_END); // seek to end of file
   size = ftell(input); // Get size of current pointer 
   fseek(input, 0, SEEK_SET); // seek back to beginning of file
   short *filebuf  = (short *)malloc(size * sizeof(short));
   if (filebuf == NULL) {
      perror("malloc");
      exit(1);
   }

   /* Read header of input to array of Char. */
   unsigned char buffer[HEADER_SIZE];
   int readHeader = fread(buffer, sizeof(char), HEADER_SIZE, input);
   if (readHeader == 0) { //Error or empty file.
      if (ferror(input) != 0) {
         printf("Error: Incorrect/empty file\n");
         exit(1);
      }
   }
   
   /* Write header from buffer to output file. */
   if((fwrite(buffer, sizeof(char), HEADER_SIZE, output)) != HEADER_SIZE) {
        if(ferror(output)) {
            perror("fwrite");
        } else {
            fprintf(stderr, "Unexpected end of file \n");
        }
        exit(1);
    }

   /* Build copies of combined into filebuf. */
   unsigned short temp[2]; //left and right samples
   int i = 0;
   short combined;
   while(fread(&temp, sizeof(short), 2, input) > 0) {
     combined = (short)(temp[0] -  temp[1])/2;
     filebuf[i] = combined;
     filebuf[i+1] = combined;
     i+=2;
   } 
   
  /* Write filebuf to output file after header.  */
   if((fwrite(filebuf, sizeof(short), i, output)) != i) {
       if(ferror(output)) {
           perror("fwrite");
       } else {
           fprintf(stderr, "Unexpected end of file \n");
         }
       exit(1);
    }

   fclose(input);
   fclose(output);

   return 0;
}
