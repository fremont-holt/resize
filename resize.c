/**
 * Copies a BMP piece by piece, just because.
 */
       
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize resize infile outfile\n");
        return 1;
    }

    // Check that the first argument supplied is a float.
    char c; float f; float resize_by;
    if ( sscanf( argv[1], " %f %c", &f, &c ) == 1 )
    {
        // Check the float supplied is more than zero but less than 100
        if( f >= 0 && f <= 100 )
        {
            resize_by = f;
        }
        else
        {
            fprintf(stderr, "Resize must be greater than 0 but less than 100\n");
            return 1;
        }
    }
    else
    {
        fprintf(stderr, "Float not supplied\n");
        return 1;
    }
    
    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];

    // open input file 
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
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

    // Store the old padding
    int old_padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // Store the width of the original image
    int old_width = bi.biWidth;
    int old_height = bi.biHeight;

    // Resize the height and width of the new image
    bi.biWidth = bi.biWidth * resize_by;
    bi.biHeight = bi.biHeight * resize_by;
    
    // determine new padding for scanlines
    int new_padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // Calculate the size of the new image
    bi.biSizeImage = ( ( sizeof( RGBTRIPLE ) * bi.biWidth ) + new_padding ) * abs( bi.biHeight );
    bf.bfSize = bi.biSizeImage + sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER );

    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(old_height); i < biHeight; i++)
    {
        // hold pixel values of row
        RGBTRIPLE row[ bi.biWidth ];
        
        // hold current row index
        int array_pos = 0;
        
        // iterate over pixels in scanline
        for (int j = 0; j < old_width; j++)
        {
            // temporary storage
            RGBTRIPLE triple;
    
            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
            
            // hold this value however many times we are resizing by
            for( int k = 0; k < resize_by; k++ )
            {
                // check to make sure not hitting outside of index
                if( array_pos == bi.biWidth )
                {
                    break;
                }
                
                // Check that color is one we want to store in our row
                if( ( j % (int)ceil( 1 / resize_by ) ) == 0 )
                {
                    row[array_pos] = triple;
                    array_pos++;
                }
            }
        
        }

        // skip over padding, if any
        fseek(inptr, old_padding, SEEK_CUR);

        // Check that the row is one we want to print
        if( ( i % (int)ceil( 1 / resize_by ) ) == 0 )
        {
            // repeat the row for the amount of times we are resizing by
            for( int k = 0; k < resize_by; k++ )
            {
            
                for( int p = 0; p < bi.biWidth; p++ )
                {
                    // temporary storage
                    RGBTRIPLE triple;
                    
                    triple = row[p];
                    
                    // write RGB triple to outfile
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }
            
                // then add it back (to demonstrate how)
                for (int l = 0; l < new_padding; l++)
                {
                    fputc(0x00, outptr);
                }   
                
            }
        }
        
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
