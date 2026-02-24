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
	int row;

	// You need to parallelise this operation.
	#pragma omp parallel for
	for( row=0; row<img->size; row++ ) {
		int col;
		for( col=0; col<img->size; col++ ) {
			img->pixels[row][col] = ( img->pixels[row][col]>127 ? 255 : 0 );
		}
	}
	
	// You must call this function to save your final image.
	writeThresholdImage( img );
}

// Flips the image vertically, and outputs to a new .pgm file.
void saveFlippedImage( struct Image *img )
{
	// Your parallel implementation should go here.

	int row;

	// Parallelise over the top half of rows only. Each row-pair is independent.
	#pragma omp parallel for
	for( row=0; row<img->size/2; row++ ) {
		int col;
		for( col=0; col<img->size; col++ ) {
			// Swap using a temp variable, declared locally so it is private.
			int temp = img->pixels[row][col];
			img->pixels[row][col] = img->pixels[img->size-1-row][col];
			img->pixels[img->size-1-row][col] = temp;
		}
	}

	// You must call this function to save your final image.
	writeFlippedImage( img );
}

// Outputs an image that highights edges in the original.
void saveEdgeImage( struct Image *img )
{
	// For the edge image, each pixel at (row,col) should be replaced with the value returned by
	// edgeValue(row,col,img), which is defined in cwk_extra.h and returns higher values near edges.
	// Note edgeValue() reads pixels at: [row-1][col], [row+1][col], [row][col-1], and [row][col+1].

	// Allocate a temporary grid to store edge values, as per the copy approach in work1_sol4.c.
	int **edgePixels;
	allocSquareGrid( &edgePixels, img->size );

	int row;

	// Red-black pass 1: Process "red" cells where (row+col) is even.
	#pragma omp parallel for
	for( row=0; row<img->size; row++ ) {
		int col;
		for( col=0; col<img->size; col++ ) {
			if( (row+col)%2 == 0 )
				edgePixels[row][col] = edgeValue( row, col, img );
		}
	}

	// Red-black pass 2: Process "black" cells where (row+col) is odd.
	#pragma omp parallel for
	for( row=0; row<img->size; row++ ) {
		int col;
		for( col=0; col<img->size; col++ ) {
			if( (row+col)%2 == 1 )
				edgePixels[row][col] = edgeValue( row, col, img );
		}
	}

	// Copy the edge values back into the image, as per work1_sol4.c pattern.
	#pragma omp parallel for
	for( row=0; row<img->size; row++ ) {
		int col;
		for( col=0; col<img->size; col++ ) {
			img->pixels[row][col] = edgePixels[row][col];
		}
	}

	// Free the temporary grid.
	int i;
	for( i=0; i<img->size; i++ ) free( edgePixels[i] );
	free( edgePixels );

	// You must call this function to save your final image.
	writeEdgeImage( img );
}

// Constructs and outputs a histogram containing the number of each greyscale value in the image.
void generateHistogram( struct Image *img )
{
	// Initialise the histogram to zero ("calloc" rather than "malloc"). You do not need to parallelise this initialisation.
	int *hist = (int*) calloc( MAXVALUE, sizeof(int) );

	// Loop through all pixels and add to the relevant histogram bin.
	int row;

	// Parallelise outer loop. Use atomic to protect hist[val]++ (work1_sol6.c pattern).
	#pragma omp parallel for
	for( row=0; row<img->size; row++ ) {
		int col;
		for( col=0; col<img->size; col++ )
		{
			int val = img->pixels[row][col];
			if( val>=0 && val<=MAXVALUE )		// Check that the value is in the valid range before updating the histogram.
			#pragma omp atomic
				hist[val]++;
		}
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
