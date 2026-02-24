//
// COMP/XJCO3221 Parallel Computation Coursework 1: OpenMP / Shared Memory Parallelism.
//


//
// Includes.
//

// Standard includes.
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// The OMP library.
#include <omp.h>


//
// The include file for this coursework. You may inspect this file, but should NOT alter it, as it will be
// replaced with a different version for assessment.
//
#include "cwk1_extra.h"
//
// This file includes the following definition for the Image struct that the rest of the code uses.
//
// struct Image {
// 	int size;			// Width and height in pixels for a square image.
// 	int** pixels;		// pixels[row][col] for the pixel with the given row and column.
// };
//
// It also includes the define MAXVALUE, the maximum grey scale value, which takes the value 255.
//


//
// The following functions are called by main() for each of the options.
//

// Constructs a thresholded image in place, and saves as a new .pgm file.
void saveThresholdImage( struct Image *img )
{
	int row, col;

	// You need to parallelise this operation.
	for( row=0; row<img->size; row++ )
		for( col=0; col<img->size; col++ )
			img->pixels[row][col] = ( img->pixels[row][col]>127 ? 255 : 0 );

	// You must call this function to save your final image.
	writeThresholdImage( img );
}

// Flips the image vertically, and outputs to a new .pgm file.
void saveFlippedImage( struct Image *img )
{
	// Your parallel implementation should go here.

	// You must call this function to save your final image.
	writeFlippedImage( img );
}

// Outputs an image that highights edges in the original.
void saveEdgeImage( struct Image *img )
{
	// For the edge image, each pixel at (row,col) should be replaced with the value returned by
	// edgeValue(row,col,img), which is defined in cwk_extra.h and returns higher values near edges.
	// Note edgeValue() reads pixels at: [row-1][col], [row+1][col], [row][col-1], and [row][col+1].

	// Your parallel implementation should go here.

	// You must call this function to save your final image.
	writeEdgeImage( img );
}

// Constructs and outputs a histogram containing the number of each greyscale value in the image.
void generateHistogram( struct Image *img )
{
	// Initialise the histogram to zero ("calloc" rather than "malloc"). You do not need to parallelise this initialisation.
	int *hist = (int*) calloc( MAXVALUE, sizeof(int) );

	// Loop through all pixels and add to the relevant histogram bin.
	int row, col;

	// You need to parallelise this operation.
	for( row=0; row<img->size; row++ )
		for( col=0; col<img->size; col++ )
		{
			int val = img->pixels[row][col];
			if( val>=0 && val<=MAXVALUE )		// Check that the value is in the valid range before updating the histogram.
				hist[val]++;
		}
	
	// Save the histogram to file. There is a Python script you can use to visualise the results from this file.
	saveHistogram( hist );

	// Free up the memory allocated for the histogram.
	free( hist );
}


//
// You should not modify the code in main(), but should understand how it works.
//
int main( int argc, char **argv )
{
    //
    // Parse command line arguments. Requires a filename and an option number.
    //

    // Make sure we have exactly 2 command line arguments, which, plus the executable name, means 'argc' should be exactly 3.
    if( argc != 3 )
    {
        printf( "Call with the name of the image file to read, and a single digit for the operation to perform:\n" );
		printf( "(1) Save a thresholded version of the image;\n(2) Save a vertically flipped image;\n" );
		printf( "(3) Save a edge image that attempts to highlight edges in the original; or\n" );
		printf( "(4) Save a histogram of grey scale values in the image.\n" );

        return EXIT_FAILURE;
    }

    // Convert to an option number, and ensure it is in the valid range. Note argv[0] is the executable name.
    int option = atoi( argv[2] );
    if( option<1 || option>4 )
    {
        printf( "Option number '%s' invalid.\n", argv[2] );
        return EXIT_FAILURE;
    }
 
	// Attempts to open the file and return a structure for the image.
	struct Image img;
	if( readImage(argv[1],&img)==-1 ) return EXIT_FAILURE;
	printf( "Loaded a square %ix%i image with a maximum greyscale value of %i.\n", img.size, img.size, MAXVALUE );

	//
	// Perform the action based on the option provided on the command line.
	//
	printf( "Performing option '%i' using %i OpenMP thread(s).\n", option, omp_get_max_threads() );

	switch( option )
	{
		case 1 : saveThresholdImage( &img ); break;
		case 2 : saveFlippedImage  ( &img ); break;
		case 3 : saveEdgeImage     ( &img ); break;
		case 4 : generateHistogram( &img ); break;
		default:
			return EXIT_FAILURE;
	}

	//
	// Finalise.
	//

	// Release memory allocated for the image.
	freeImage( &img );

    return EXIT_SUCCESS;
}
