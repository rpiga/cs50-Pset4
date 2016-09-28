#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <cs50.h>

#include "bmp.h"

int main(int argc, char* argv[])
{
    // Ensure proper usage
    if (argc != 4)
    {
        printf("Usage: %s n infile outfile\n", argv[0]);
        return 1;
    }
    
    int n = atoi(argv[1]);
    if (n < 1 || n > 100)
    {
        printf("n must be a value between 0 and 100\n");
        return 5;
    }

    // remember filenames
    char* infile = argv[2];
    char* outfile = argv[3];
    
    // open input file 
    FILE* inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE* outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }
    
    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }    

    // determine padding for scanlines
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // Create headers for outfile
    BITMAPFILEHEADER bf_out;
    BITMAPINFOHEADER bi_out;
    
    bf_out = bf;
    bi_out = bi;
    
/* 
    Headers to modify:
    bfSize   54 + ((W + padding)*3) * abs(H)
    biWidth  x n
    biHeight x n
    biSizeImage
    ? biXPelsPerMeter
    ? biYPelsPerMeter
*/

    bi_out.biHeight = bi.biHeight * n;
    bi_out.biWidth = bi.biWidth * n;

    // determine padding for scanlines in the output file
    int padding_out = (4 - (bi_out.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    bi_out.biSizeImage = ((bi_out.biWidth * sizeof(RGBTRIPLE)) + padding_out) * abs(bi_out.biHeight);
    bf_out.bfSize = bi_out.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    
    // write outfile's BITMAPFILEHEADER
    fwrite(&bf_out, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi_out, sizeof(BITMAPINFOHEADER), 1, outptr);

    fpos_t position;
    int f = (n - 1);
    
    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi_out.biHeight); i < biHeight; i++)
    {
        fgetpos(inptr, &position);
        
        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
            // temporary storage
            RGBTRIPLE triple;
            
            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
            
            for (int k = 0; k < n; k++)
                fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
        }
        
        // skip over padding, if any
        fseek(inptr, padding, SEEK_CUR);
        
        // Add padding to outfile
        for (int p = 0; p < padding_out; p++)
            fputc(0x00, outptr);
        
        if (f > 0)
        {
            fsetpos(inptr, &position);
            f--;
        }
        else
            f = (n - 1);
    }

    
    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);
    
    // End.
    return 0;
}