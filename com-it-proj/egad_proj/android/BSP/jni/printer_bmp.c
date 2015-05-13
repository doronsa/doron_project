#include <stdio.h>
#include <stdlib.h>
#include "lib_bmp.h"


void CreateBuffer(int width , int height , unsigned char* dots , char** buffer , int* idx)
{
   (*buffer) = (char*)malloc (sizeof (char) * width*height + height + 100);
    int index = 0;
   (*buffer)[index++] = (char)0x1B;
   (*buffer)[index++] = '3';
   (*buffer)[index++] =  (char)24;
   int offset = 0;

              while (offset < height)
              {
            	  (*buffer)[index++] =(char)0x1B;
            	  (*buffer)[index++]='*';         // bit-image mode
            	  (*buffer)[index++]=(char)33;    // 24-dot double-density
            	  (*buffer)[index++]= width & 0x00FF;  // width low byte
            	  (*buffer)[index++]= (width >> 8) & 0x00FF;  // width high byte
                //  printf("%d %d\n",width,height);

            	  int x;
                  for ( x = 0; x < width; ++x)
                  {
                      int k;
                	  for ( k = 0; k < 3; ++k)
                      {
                          char slice = 0;
                          int b;
                          for ( b = 0; b < 8; ++b)
                          {
                              int y = (((offset / 8) + k) * 8) + b;
                              // Calculate the location of the pixel we want in the bit array.
                              // It'll be at (y * width) + x.
                              int i = (y * width) + x;

                              // If the image is shorter than 24 dots, pad with zero.
                              int v = 0;
                              if (i < width*height)
                              {
                            	//  printf("[%d] = %d \n" ,i ,dots[i]  );
                            	  v = dots[i];

                              }
                              slice |= (char)((v ? 1 : 0) << (7 - b));
                          }

                          (*buffer)[index++]=slice;
                          //printf("[%d] = %d \n" ,index , buffer[index] );
                      }
                  }
                  offset += 24;
                  (*buffer)[index++]=(char)0x0A;
              }
              // Restore the line spacing to the default of 30 dots.
           //   bw.Write((char)0x1B);
           //   bw.Write('3');
            //  bw.Write((byte)30);

              (*buffer)[index++]=((char)0x0A);
              (*buffer)[index++]=((char)0x0A);
              *idx = index;

              //puts("END 3 \n");
         //     int var;



}


void ReadBMP(int* width ,int* height , unsigned char ** dots , int* index )
{

	  int threshold = 127;
	  *index = 0;
	  double multiplier = 450; // 570 this depends on your printer model. for Beiyang you should use 1000
	  double scale = (double)(multiplier / (double)*width);
	  scale = 1;
          int xheight = (int)(*height * scale);
	  int xwidth = (int)(*width * scale);
	  int dimensions = xwidth * xheight;

	  *height = xheight;
	  *width = xwidth;


	 // unsigned char dots[dimensions];
	  (*dots) = (unsigned char*)malloc (sizeof (char) * dimensions);

        printf(" %d %d %d \n" ,scale ,*height,*width);



   int y;
    for ( y = 0; y < xheight; y++)
                  {
    	              int x;
    	              int  _y = (int)(y / scale);
                      for ( x = 0; x < xwidth; x++)
                      {
                          int  _x = (int)(x / scale);


                          int luminance = (int)((int) GetPixelValue(_y,_x,0) * 0.3 + (int)GetPixelValue(_y,_x,1) * 0.59 + (int)GetPixelValue(_y,_x,2) * 0.11);
                          (*dots)[*index] = (luminance < threshold);
                         // printf(" [%d] = %d \n" ,*index ,dots[*index]);
                          *index = *index + 1;
                      }

                  }



    //return data;
}

int CreatePrinterBuffer(char* fileName , char** buffer , int* index)
{
	// char* buffer  = {0};
	 unsigned char* dots = {0} ;


	 int width,height;
	 int res = LoadBMPFile(fileName, &width, &height);
	 if(res == 0)
	     {
		   ReadBMP(&width,&height,&dots , index);
		   CreateBuffer(width , height , dots , buffer , index );
		   if(dots)
		     free(dots);

	     }
     return res;
}


int main1(void) {

    int index,var;
    char* buffer = {0};
	CreatePrinterBuffer("/home/user/hello.bmp" ,  &buffer , &index);

	for (var = 0; var < index; ++var) {
			 	                   printf("\\x%02X", buffer[var] & 0xFF);
			 	  			 }

	free(buffer);
	return EXIT_SUCCESS;
}
