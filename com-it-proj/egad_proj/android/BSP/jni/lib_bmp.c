#define _CRT_SECURE_NO_DEPRECATE

#include "lib_bmp.h"

#include <stdio.h>
#include <stdlib.h>

#define PATH_ERROR   -10
#define FORMAT_ERROR -11


/* Prototype declartions for input functions.  GetByte() returns a 1-byte sequence as an int,
   GetWord() returns a 2-byte sequence as an int, GetDoubleWord() returns a 4-byte sequence as an int.
   These functions assume little-endianness of file
*/
static int GetByte(FILE *fp);
static int GetWord(FILE *fp);
static int GetDoubleWord(FILE *fp);

/* This is used for storing file data for a BMP file loaded using the LoadBMPFile function */
static Colour internalBitmap[MAXH][MAXW];
static int loadSuccess = 0;

int LoadBMPFile(char* filename, int* width, int* height)
{
	/* For reading data from the file */
	int colourDepth, compression, fileSize;
	int x, y;

	/* Open BMP file */
	FILE *fptr;
	fptr = fopen(filename, "rb");
	if (fptr == NULL) {
		printf("Could not open input file: \"%s\"\n", filename);
		return PATH_ERROR;
	}

	/* Read file size */
	(void)fseek(fptr, 2, SEEK_SET);
	fileSize = GetDoubleWord(fptr);

	/* Read width and height of image */
	(void)fseek(fptr, 18, SEEK_SET);
	*width = GetDoubleWord(fptr);
	(void)fseek(fptr, 22, SEEK_SET);
	*height = GetDoubleWord(fptr);

	/* Read colour depth */
	(void)fseek(fptr, 28, SEEK_SET);
	colourDepth = GetWord(fptr);

	/* Read compression */
	(void)fseek(fptr, 30, SEEK_SET);
	compression = GetDoubleWord(fptr);

	/* Ensure a 24-bit, and therefore uncompressed BMP file */
	if ((colourDepth != 24) || (compression != 0)) {
		printf("Not a valid 24-bit BMP file.\n");
		printf("Colour depth should be 24 - it is %d.\n", colourDepth);
		printf("Compression should be 0 - it is %d.\n", compression);

		return FORMAT_ERROR;
	}

	/* Read the pixel information */
	(void)fseek(fptr, 54, SEEK_SET);

	/* Ensure memory is adequate */
	if ((*height >= MAXH) || (*width >= MAXW)) {
		printf("Not enough memory for a %dx%d image.\n", MAXW, MAXH);
		printf("Increase the values of MAXW and MAXH in LibBMP.h\n");
		return FORMAT_ERROR;

	}

	for(y = *height-1; y >= 0; y--) {
		for(x = 0; x < *width; x++) {
			internalBitmap[y][x].blue = (byte)GetByte(fptr);
			internalBitmap[y][x].green = (byte)GetByte(fptr);
			internalBitmap[y][x].red = (byte)GetByte(fptr);
		}
		/* Realign the file pointer, scan lines are word aliged */
		(void)fseek(fptr, *width % 4, SEEK_CUR);
	}

	printf("Successfully loaded bitmap information:\n");
	printf("  - Filename: \"%s\"\n", filename);
	printf("  - Width: %d\n", *width);
	printf("  - Height: %d\n", *height);
	printf("  - Bytes read: %d\n\n", fileSize);
	loadSuccess = 1;

	return 0;
}

/* channel 0, 1, 2 = red, green, blue */
int GetPixelValue(int row, int col, int channel)
{
	if (channel == 0)
		return internalBitmap[row][col].red;
	else if (channel == 1)
		return internalBitmap[row][col].green;
	else
		return internalBitmap[row][col].blue;
}

/* channel 0, 1, 2 = red, green, blue */
void SetPixelValue(int value, int row, int col, int channel)
{
	byte val = (byte)value;
	if (channel == 0)
		internalBitmap[row][col].red = val;
	else if (channel == 1)
		internalBitmap[row][col].green = val;
	else
		internalBitmap[row][col].blue = val;
}

void SaveBMPFile(char* filename, int width, int height)
{
	/* BMP file format header, for uncompressed 24-bit colour */
	byte header[54] = {	0x42,0x4D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,0x00,0x28,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x18,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00,0xC2,0x1E,0x00,0x00,0xC2,0x1E,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00 };

	FILE *fptr;
	byte* fileData;

	int aligned, padding, totalSize;
	int i;
	int x, y;
	int writeIndex = 54;

	/* Ensure a file has been loaded prior to this call */
	if (!loadSuccess) {
		printf("BMP file must be loaded before saving.\n");
		return FORMAT_ERROR;
	}

	/* Calculate total size of picture, taking into account padding */
	aligned = 4 - (width * 3) % 4;
	padding = (aligned == 4) ? 0 : aligned;
	totalSize = 54 + (((width * 3) + padding) * height);

	/* Allocate room to store filedata */
	fileData = (byte*)malloc((size_t)totalSize);
	if (fileData == NULL) {
		printf("Could not allocate sufficient memory.\n");
		return FORMAT_ERROR;
	}

	/* Initialise first 54 bytes, set total size and width and height correctly */
	for (i = 0; i < 54; i++) {
		fileData[i] = header[i];
	}
	/* File size */
	fileData[2] = (byte)(totalSize & 0xFF);
	fileData[3] = (byte)((totalSize >> 0x08) & 0xFF);
	fileData[4] = (byte)((totalSize >> 0x10) & 0xFF);
	fileData[5] = (byte)((totalSize >> 0x18) & 0xFF);
	/* Width */
	fileData[18] = (byte)(width & 0xFF);
	fileData[19] = (byte)((width >> 0x08) & 0xFF);
	fileData[20] = (byte)((width >> 0x10) & 0xFF);
	fileData[21] = (byte)((width >> 0x18) & 0xFF);
	/* Height */
	fileData[22] = (byte)(height & 0xFF);
	fileData[23] = (byte)((height >> 0x08) & 0xFF);
	fileData[24] = (byte)((height >> 0x10) & 0xFF);
	fileData[25] = (byte)((height >> 0x18) & 0xFF);

	/* Copy bitmap data */
	for(y=height-1; y >=0; y--) {
		for(x=0; x < width; x++) {
			fileData[writeIndex++] = internalBitmap[y][x].blue;
			fileData[writeIndex++] = internalBitmap[y][x].green;
			fileData[writeIndex++] = internalBitmap[y][x].red;
		}
		writeIndex += width%4;
	}

	/* Write to file */
	fptr = fopen(filename, "wb");
	if (fptr == NULL) {
		printf("Could not open output file: \"%s\"\n", filename);
		return PATH_ERROR;
	}

	/* Write data */
	(void)fwrite(fileData, 1, (size_t)totalSize, fptr);

	/* Close files */
	(void)fclose(fptr);

	/* Release memory */
	free(fileData);

	printf("Successfully saved bitmap information:\n");
	printf("  - Filename: \"%s\"\n", filename);
	printf("  - Width: %d\n", width);
	printf("  - Height: %d\n", height);
	printf("  - Bytes written: %d\n\n", totalSize);
}

void LoadColourArray(char* filename, Colour bitmap[MAXH][MAXW], int* width, int* height)
{
	/* For reading data from the file */
	int colourDepth, compression, fileSize;
	int x, y;

	/* Open BMP file */
	FILE *fptr;
	fptr = fopen(filename, "rb");
	if (fptr == NULL) {
		printf("Could not open input file: \"%s\"\n", filename);
		return FORMAT_ERROR;
	}

	/* Read file size */
	(void)fseek(fptr, 2, SEEK_SET);
	fileSize = GetDoubleWord(fptr);

	/* Read width and height of image */
	(void)fseek(fptr, 18, SEEK_SET);
	*width = GetDoubleWord(fptr);
	(void)fseek(fptr, 22, SEEK_SET);
	*height = GetDoubleWord(fptr);

	/* Read colour depth */
	(void)fseek(fptr, 28, SEEK_SET);
	colourDepth = GetWord(fptr);

	/* Read compression */
	(void)fseek(fptr, 30, SEEK_SET);
	compression = GetDoubleWord(fptr);

	/* Ensure a 24-bit, and therefore uncompressed BMP file */
	if ((colourDepth != 24) || (compression != 0)) {
		printf("Not a valid 24-bit BMP file.\n");
		printf("Colour depth should be 24 - it is %d.\n", colourDepth);
		printf("Compression should be 0 - it is %d.\n", compression);
		return FORMAT_ERROR;
	}

	/* Read the pixel information */
	(void)fseek(fptr, 54, SEEK_SET);

	/* Ensure memory is adequate */
	if ((*height >= MAXH) || (*width >= MAXW)) {
		printf("Not enough memory for a %dx%d image.\n", MAXW, MAXH);
		printf("Increase the values of MAXW and MAXH in LibBMP.h\n");
		exit(EXIT_FAILURE);
	}

	for(y = *height-1; y >= 0; y--) {
		for(x = 0; x < *width; x++) {
			bitmap[y][x].blue = (byte)GetByte(fptr);
			bitmap[y][x].green = (byte)GetByte(fptr);
			bitmap[y][x].red = (byte)GetByte(fptr);
		}
		/* Realign the file pointer, scan lines are word aliged */
		(void)fseek(fptr, *width % 4, SEEK_CUR);
	}

	printf("Successfully loaded bitmap information:\n");
	printf("  - Filename: \"%s\"\n", filename);
	printf("  - Width: %d\n", *width);
	printf("  - Height: %d\n", *height);
	printf("  - Bytes read: %d\n\n", fileSize);
}

void SaveColourArray(char* filename, Colour bitmap[MAXH][MAXW], int width, int height)
{
	/* BMP file format header, for uncompressed 24-bit colour */
	byte header[54] = {	0x42,0x4D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,0x00,0x28,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x18,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00,0xC2,0x1E,0x00,0x00,0xC2,0x1E,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00 };

	FILE *fptr;
	byte* fileData;

	int aligned, padding, totalSize;
	int i;
	int x, y;
	int writeIndex = 54;

	/* Calculate total size of picture, taking into account padding */
	aligned = 4 - (width * 3) % 4;
	padding = (aligned == 4) ? 0 : aligned;
	totalSize = 54 + (((width * 3) + padding) * height);

	/* Allocate room to store filedata */
	fileData = (byte*)malloc((size_t)totalSize);
	if (fileData == NULL) {
		printf("Could not allocate sufficient memory.\n");
		exit(EXIT_FAILURE);
	}

	/* Initialise first 54 bytes, set total size and width and height correctly */
	for (i = 0; i < 54; i++) {
		fileData[i] = header[i];
	}
	/* File size */
	fileData[2] = (byte)(totalSize & 0xFF);
	fileData[3] = (byte)((totalSize >> 0x08) & 0xFF);
	fileData[4] = (byte)((totalSize >> 0x10) & 0xFF);
	fileData[5] = (byte)((totalSize >> 0x18) & 0xFF);
	/* Width */
	fileData[18] = (byte)(width & 0xFF);
	fileData[19] = (byte)((width >> 0x08) & 0xFF);
	fileData[20] = (byte)((width >> 0x10) & 0xFF);
	fileData[21] = (byte)((width >> 0x18) & 0xFF);
	/* Height */
	fileData[22] = (byte)(height & 0xFF);
	fileData[23] = (byte)((height >> 0x08) & 0xFF);
	fileData[24] = (byte)((height >> 0x10) & 0xFF);
	fileData[25] = (byte)((height >> 0x18) & 0xFF);

	/* Copy bitmap data */
	for(y=height-1; y >=0; y--) {
		for(x=0; x < width; x++) {
			fileData[writeIndex++] = bitmap[y][x].blue;
			fileData[writeIndex++] = bitmap[y][x].green;
			fileData[writeIndex++] = bitmap[y][x].red;
		}
		writeIndex += width%4;
	}

	/* Write to file */
	fptr = fopen(filename, "wb");
	if (fptr == NULL) {
		printf("Could not open output file: \"%s\"\n", filename);
		exit(EXIT_FAILURE);
	}

	/* Write data */
	(void)fwrite(fileData, 1, (size_t)totalSize, fptr);

	/* Close files */
	(void)fclose(fptr);

	/* Release memory */
	free(fileData);

	printf("Successfully saved bitmap information:\n");
	printf("  - Filename: \"%s\"\n", filename);
	printf("  - Width: %d\n", width);
	printf("  - Height: %d\n", height);
	printf("  - Bytes written: %d\n\n", totalSize);
}


static int GetByte(FILE *fp)
{
	int w;
	w =  (int) (fgetc(fp) & 0xFF);
	return(w);
}

static int GetWord(FILE *fp)
{
	int w;
	w =  (int) (fgetc(fp) & 0xFF);
	w |= ((int) (fgetc(fp) & 0xFF) << 0x08);
	return(w);
}

static int GetDoubleWord(FILE *fp)
{
	int dw;
	dw =  (int) (fgetc(fp) & 0xFF);
	dw |= ((int) (fgetc(fp) & 0xFF) << 0x08);
	dw |= ((int) (fgetc(fp) & 0xFF) << 0x10);
	dw |= ((int) (fgetc(fp) & 0xFF) << 0x18);
	return(dw);
}
