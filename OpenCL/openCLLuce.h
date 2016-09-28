// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#pragma once

#include <CL/opencl.h>

class Texture;

class COpenCLLuce
{
public:
	COpenCLLuce();
	~COpenCLLuce();

	bool Init(cl_context context);
	void Destroy();

	void Do(cl_command_queue queue, Texture* pSrc,Texture* pDst,int lx,int ly);
private:
	cl_kernel kernel;
	cl_program program;
};