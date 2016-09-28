/**
 * recover.c
 *
 * Computer Science 50
 * Problem Set 4
 *
 * Recovers JPEGs from a forensic image.
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

// Constants
#define BLOCK 512
#define INFILE "card.raw"

int main(int argc, char* argv[])
{
    char filename[sizeof("NNN.jpg")], test[6];
    uint8_t rawin[BLOCK] = { 0 };
    int jpg_counter = 0, offset = 0*BLOCK;
    int jpg_start = 0;
    int soi = 0;

    // Open input file
    FILE* file = fopen(INFILE, "r");
    if (file == NULL)
    {
        printf("Could not open %s.\n", INFILE);
        return 1;
    }
    fseek(file, offset, SEEK_SET);
    
    FILE* output = NULL;
    
    
    // Read blocks
    while ( fread(&rawin, BLOCK, 1, file) == 1)
    {
        //printf("%d", rawin[0]);
        
        sprintf(test, "%02x%02x%02x", rawin[0], rawin[1], rawin[2]);
        
        if ( strcmp("ffd8ff", test) == 0 )
        {
            if (output) 
            {
                fclose(output);
                jpg_counter++;
            }


            // Open output file
            sprintf(filename, "%03d.jpg", jpg_counter);
            output = fopen(filename, "w");
            if (output == NULL)
            {
                printf("Could not open %s.\n", filename);
                fclose (file);
                return 2;
            }
            soi ++;
            jpg_start = 1;
        }
        
        if (jpg_start)
        {
            fwrite(&rawin, BLOCK , 1, output);
        }
    }
    
    // Close files
    fclose (output);
    fclose (file);
    
    // End.
    return 0;
}