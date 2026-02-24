//
// Extras file for the OpenMP coursework. Do not change anything in this file,
// as it will be replaced with a different version for assessment.
//


//
// Includes.
//
#include <stdio.h>
#include <stdlib.h>


//
// Defines.
//
#define MAXVALUE 255


//
// The structure used to store the image and related information.
//
struct Image {
	int size;			// Width and height in pixels for a square image.
	int** pixels;		// pixels[row][col] for the pixel with the given row and column.
};


//
// Routines for reading and writing .pgm files.
//

// Returns an integer that is high around edges, but no more than MAXVALUE.
// Reads pixels above, below, to the left, and to the right, of [row][col].
// Both row and col should be >=0 and less than 'img->size'.
int edgeValue( int row, int col, struct Image *img )
{
	// Boundary values are always returned as black.
	if( row==0 || col==0 || row==img->size-1 || col==img->size-1 ) return 0;

	// Approximate the gradient in x and y-directions.
	int dy = img->pixels[row-1][col  ] - img->pixels[row+1][col  ];
	int dx = img->pixels[row  ][col-1] - img->pixels[row  ][col+1];

	// The norm of the gradient, i.e., 'how much the figure is changing by.'
	int norm = sqrt( dx*dx + dy*dy );
	
	return ( norm>MAXVALUE ? MAXVALUE : norm );
}

// Allocates memory for a square, NxN array of integers.
void allocSquareGrid( int ***grid, int N )
{
	int i;

	*grid = (int**) malloc( sizeof(int*) * N );
	for( i=0; i<N; i++ )
		(*grid)[i] = (int*) malloc( sizeof(int) * N );
}

// Reads in a image file in PGM format (ASCII, greyscale) with the given filename, and fills in the 
// passed pointer to the Image struct. This will allocate memory for the pixels, which will need to
// be freed after usage. Returns -1 after printing an error message if it fails.
int readImage( char *fname, struct Image *img )
{
	int row, col;			// Rows and columns of pixels in the image.

	// Try to open the image file.
	FILE *file = fopen( fname, "r" );
	if( !file )
	{
		printf( "Could not open image '%s' file for reading.\n", fname );
		return -1;
	}

	// The first line should be "P2"; provide some basic error checking.
	char header[10];
	fscanf( file, "%s", header );
	if( header[0]!='P' || header[1]!='2' )
	{
		printf( "File not greyscale ASCII pgm.\n" );
		return -1;
	}

	// The next two numbers are the image dimensions, which need to be the same for this coursework.
	int width, height;
	fscanf( file, "%i %i", &width, &height );
	if( width != height )
	{
		printf( "Image is not square (height=%i but width=%i).\n", height, width );
		return -1;
	}
	img->size = width;

	// Can now allocate the two-dimensional array.
	allocSquareGrid( &img->pixels, img->size );
	if( !img->pixels )
	{
		printf( "Failed to allocate memeory for the pixels.\n" );
		return -1;
	}

	// The next number is the maximum allowed value in the grey scale, i.e. the value corresponding to 'white.'
	int maxValue;
	fscanf( file, "%i", &maxValue );
	if( maxValue != MAXVALUE )
	{
		printf( "Maximum grey scale value of the image is not %i.\n", MAXVALUE );
		return -1;
	}

	// Read in all of the pixels, which are arranged in blocks of pixels for each row, hence 'row' is the inner loop.
	for( col=0; col<img->size; col++ )
		for( row=0; row<img->size; row++ )
			fscanf( file, "%i", &img->pixels[col][row] );

	// Close the file.
	fclose(file);	

	// Return with a '0', meaning 'success'.
	return 0;
}

// Frees up the resources associated with the passed image. Need to know the number of rows.
void freeImage( struct Image *img )
{
	int row;

	// Free up the memory for each row first of all.
	for( row=0; row<img->size; row++ ) free( img->pixels[row] );

	// Now free up the memory for all of the row pointers.
	free( img->pixels );
}

// Saves the given image to the given filename, in PGM format.
void writeImage( char *fname, struct Image *img )
{
	int row, col;

	// Try to open the file for writing.
	FILE *file = fopen( fname, "w" );
	if( !file )
	{
		printf( "Could not open the file '%s' for writing.\n", fname );
		return;
	}

	// The first lines specify the format (lien 1), width and height (line 2), and maximum greyscale value (line 3).
	fprintf( file, "P2\n" );
	fprintf( file, "%i %i\n", img->size, img->size );
	fprintf( file, "%i\n", MAXVALUE );

	// Now output all of the geyscale values, pixel by pixel, with one line per row.
	for( col=0; col<img->size; col++ )
	{
		for( row=0; row<img->size; row++ ) fprintf( file, "%i ", img->pixels[col][row] );
		fprintf( file, "\n" );
	}

	// Message to stdout.
	printf( "Saving image to file '%s'.\n", fname );

	// Close the file.
	fclose(file);
}

//
// Output routines for this coursework.
//

// Outputs the thresholded image using a hard-coded filename.
void writeThresholdImage( struct Image *img ) { writeImage("threshold.pgm",img); }

// Outputs the vertically flipped image using a hard-coded filename.
void writeFlippedImage( struct Image *img ) { writeImage("flipped.pgm",img); }

// Outputs the edge-detection image using a hard-coded filename.
void writeEdgeImage( struct Image *img ) { writeImage("edge.pgm",img); }

// Saves a histogram to file in a format that can be read by the associated Python script.
void saveHistogram( int *hist )
{
	int bin;

	// Try to open the file.
	FILE *file = fopen( "histogram.dat", "w" );
	if( !file )
	{
		printf( "Could not open the file 'histogram.dat' to output the histogram.\n" );
		return;
	}

	// Output one bin at a time.
	for( bin=0; bin<MAXVALUE; bin++ )
		fprintf( file, "%i\t%i\n", bin, hist[bin] );
	
	// Message to stdout.
	printf( "Output greyscale histogram to 'histogram.dat'. Use the provided Python script to view.\n" );

	// Close the file.
	fclose( file );
}
