// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <CL/opencl.h>

//! 
class Texture
{
public:
	Texture();
	virtual ~Texture();

	//! Setup the image, 
	//! init the property of image and create the image data
	bool Create(short _dimx,short _dimy,short _bpp);
	//! Initialize the OpenGL Texture
	//! \param alpha In case of byte per pixel (#bpp) is 1, this parameter indicate if the texture is grayscale or alpha only
	//! \note After that the image data is destroyed
	bool Init(bool alpha = false);
	//! Set this texture for OpenGL rendering
	void SetIt() const;
	//! Set no-texture for OpenGL rendering
	static void Reset();

	//! Initialize the OpenCL memory
	//! \note It must be done after #Init
	bool InitCL(cl_context context,cl_mem_flags flags);
	//|

	short GetWidth() const { return dimx; }
	short GetHeight() const { return dimy; }
	short GetBPP() const { return bpp; }
	unsigned char* GetData() const { return bits; }
	unsigned int GetGL() const { return idogl; }
	const cl_mem& GetCL() const { return idocl; }
protected:
	//! width
	short dimx;
	//! height
	short dimy;
	//! byte per pixel, it can be:
	//! 1: grayscale or alpha only
	//! 2: grayscale with alpha
	//! 3: RGB
	//! 4: RGBA
	short bpp;
	//! image data
	unsigned char *bits;
	//! OpenGL ID
	unsigned int idogl;
	//! OpenCL ID
	cl_mem idocl;
};

#endif //__TEXTURE_H__
