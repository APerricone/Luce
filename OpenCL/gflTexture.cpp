// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#include "gflTexture.h"
#include <libgfl.h>

gflTexture::gflTexture(void)
{
}

gflTexture::~gflTexture(void)
{
}

void gflTexture::InitLib()
{
	gflLibraryInit();
}

void gflTexture::DeinitLib()
{
	gflLibraryExit();
}

bool gflTexture::Create(const char* fileName)
{
	printf("Loading \"%s\"...", fileName);
	GFL_LOAD_PARAMS load_option; 
	GFL_BITMAP *bitmap = 0; 
	GFL_FILE_INFORMATION file_info; 
	GFL_ERROR error; 

	gflGetDefaultLoadParams( &load_option ); 
	load_option.ColorModel = GFL_RGBA; 
	load_option.Origin = GFL_BOTTOM_LEFT; 

	error = gflLoadBitmap( fileName, &bitmap, &load_option, &file_info ); 
	if ( error || bitmap==NULL)
	{
		if(bitmap) gflFreeBitmap(bitmap);
		const char * str = gflGetErrorString( error ); 
		printf("\n\tERROR: gflTexture::Create(%s)\n",str);
		return false;
	}
	if( file_info.BitsPerComponent != 8 )
	{
		gflFreeBitmap(bitmap);
		printf("\n\tERROR: gflTexture::Create bit per component unsupported %i\n",file_info.BitsPerComponent);
		return false;
	}
	Texture::Create(bitmap->Width,bitmap->Height,/*bitmap->ComponentsPerPixel*/4);
	unsigned char *dest = bits; 
	GFL_UINT8* ori = bitmap->Data;
	ptrdiff_t strip = bitmap->BytesPerLine - bitmap->BytesPerPixel * bitmap->Width;
	for(short y=0;y<dimy;y++)
	{
		for(short x=0;x<dimx;x++)
		{
			switch(bitmap->ComponentsPerPixel)
			{
			case 1:
				dest[2] = dest[1] = dest[0] = ori[0];
				dest[3] = 255;
				break;
			case 3:
				dest[0] = ori[0];
				dest[1] = ori[1];
				dest[2] = ori[2];
				dest[3] = 255;
				break;
			case 4:
				break;
				dest[0] = ori[0];
				dest[1] = ori[1];
				dest[2] = ori[2];
				dest[3] = ori[3];
				break;
			}
			dest+=4;
			ori+=bitmap->BytesPerPixel;
		}
		ori += strip;
	}

	gflFreeBitmap(bitmap);
	printf("OK\n");
	return true;
}