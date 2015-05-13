#ifndef LIB_BMP_H_
#define LIB_BMP_H_
/* Colour values in the range 0-255 */
typedef unsigned char byte;

/* Bitmap data for a single pixel */
typedef struct {
	byte red;
	byte green;
	byte blue;
} Colour;

/* The maximum width and height of the image supported */
#define MAXW 1500
#define MAXH 1500

/* Function prototype declarations */
int LoadBMPFile(char *filename, int *width, int *height);
void SaveBMPFile(char *filename, int width, int height);

int GetPixelValue(int row, int col, int channel);
void SetPixelValue(int value, int row, int col, int channel);

void LoadColourArray(char *filename, Colour bitmap[MAXH][MAXW], int *width, int *height);
void SaveColourArray(char *filename, Colour bitmap[MAXH][MAXW], int width, int height);

int CreatePrinterBuffer(char* fileName , char** buffer , int* index);
#endif
