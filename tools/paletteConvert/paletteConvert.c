#include <stdio.h>

typedef struct {
	char *hex;
	char r;
	char g;
	char b;
} Swatch;

typedef struct {
	char *header;
	Swatch swatches[256];
} Palette;

int main(int argc, char *argv[])
{
	if(argc != 3) return 0;

	char *filenameIn = argv[1];
	char *filenameOut = argv[2];

	FILE *fileIn;
	FILE *fileOut;

	char readChar = 0;

	if(!fopen(argv[1], "r"))
	{
		printf("Couldn't open file %s\n", argv[1]);
		return 1;
	}

	if(!fopen(argv[2], "w+"))
	{
		printf("Couldn't open file %s\n", argv[2]);
	}

	Palette palette;

	//TODO: read all the header information
	// read by line?
	// fill the header field of the palette struct
	
	//TODO: read each line of the swatches
	// create a new swatch struct for each one
	// parse the R, G, B, and Hex values
	
	//TODO: transform the swatch values
	// calculate the new hex value and write it to the swatch
	
	//TODO: write the new file using the values in the palette struct

	fclose(fileIn);
	fclose(fileOut);

	return 0;
}
