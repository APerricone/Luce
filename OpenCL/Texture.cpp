// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#include "Texture.h"
#ifdef _WINDOWS
#include <Windows.h>
#endif
#include <gl/GL.h>
#include <stdio.h>

Texture::Texture()
{
	bits=0;
	idogl=0;
	idocl=0;
	dimx=dimy=bpp=0;
}

Texture::~Texture()
{
	if( bits ) delete [] bits;
	if( idogl ) glDeleteTextures(1,&idogl);
	if( idocl ) clReleaseMemObject(idocl);
}

bool Texture::Create(short _dimx,short _dimy,short _bpp)
{
	/*short ck;
	ck=1; while(ck<1024) { if(dimx==ck) break; ck+=ck; }
	if(dimx!=ck) { message("Texture::Create","invalid dimx(%i)",dimx); DebugBreak(); }
	ck=1; while(ck<1024) { if(dimy==ck) break; ck+=ck; }
	if(dimy!=ck) { message("Texture::Create","invalid dimy(%i)",dimy); DebugBreak(); } */
	if(_dimx==0 || _dimy==0)
	{
		printf("ERROR: Texture::Create invalid param dimensions:%1 x %1",_dimx,_dimy);
		return false;
	}
	if((_bpp<1)||(_bpp>4))
	{
		printf("ERROR: Texture::Create invalid param _bpp:%1, it can be only 1,2,3,4",bpp);
		return false;
	}
	if(_dimx!=0) dimx=_dimx;
	if(_dimy!=0) dimy=_dimy;
	if(_bpp!=0) bpp=_bpp;
	bits=new unsigned char[dimx*dimy*bpp];
	return true;
}

#pragma region OpenGL

bool Texture::Init(bool alpha)
{
	if(bits==0)
	{
		printf("ERROR: Texture::Init NULL bits");
		return false;
	}
	glGenTextures(1,&idogl);
	glBindTexture(GL_TEXTURE_2D,idogl);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	GLenum format;
	switch(bpp)
	{
	case 1: if(alpha) format=GL_ALPHA; else format=GL_LUMINANCE; break;
	case 2: format=GL_LUMINANCE_ALPHA; break;
	case 3: format=GL_RGB; break;
	case 4: format=GL_RGBA; break;
	}
	glTexImage2D(GL_TEXTURE_2D,0,bpp,dimx,dimy,0,format,GL_UNSIGNED_BYTE,bits);	
	delete [] bits; bits=0;

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) 
	{
		printf("ERROR: Texture::init OpenGL error %i\n", err);
		return false;
	}
	glBindTexture(GL_TEXTURE_2D,0);
	return true;
}

void Texture::SetIt() const
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,idogl);
}

void Texture::Reset()
{
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,0);
}

#pragma endregion

bool Texture::InitCL(cl_context context,cl_mem_flags flags)
{
	cl_int error;
	idocl = clCreateFromGLTexture2D(context,  flags,  GL_TEXTURE_2D, 0, idogl, &error);
	if(error != CL_SUCCESS) 
	{
		printf("ERROR: can't init OpenGL.(clCreateFromGLTexture2D fails:%i)\n", error);
		return false;
	}
	return true;
}
