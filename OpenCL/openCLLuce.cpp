// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#include "openCLLuce.h"
#include "Texture.h"
#include <stdio.h>

COpenCLLuce::COpenCLLuce()
{
	kernel = 0;
	program = 0;
}

COpenCLLuce::~COpenCLLuce()
{
	if( kernel ) clReleaseKernel(kernel);
	if( program ) clReleaseProgram(program);
}

bool COpenCLLuce::Init(cl_context context)
{
	printf("Initialize Luce...\n");
	printf("  Opening \"luce.cl\"...");
	FILE *f = fopen("luce.cl","rt");
	if( f == 0 )
	{
		printf("ERROR:  Unable to open file.\n");
		return false;
	}
	fseek ( f, 0L , SEEK_END ); 
	unsigned int size = ftell(f);
	char *text = new char[size+1];
	if( text == 0 )
	{
		fclose(f);
		printf("ERROR:  Unable to open file\n");
		return false;
	}
	fseek ( f, 0L , SEEK_SET ); 
	unsigned int nRead = fread(text,1,size,f);
	text[nRead] = 0;
	fclose(f);
	printf("OK\n  Compiling...");
#define CL_ERROR(msg)  if(error != CL_SUCCESS) \
						{ \
							printf("ERROR: " msg " fails: %i\n", error); \
							return false; \
						}
	cl_int error;
	program = clCreateProgramWithSource(context,1,(const char**)&text,0,&error);
	CL_ERROR("clCreateProgramWithSource");
	//
	cl_device_id  aDevices[10];
	cl_uint nDevices;
	error = clGetContextInfo(context,CL_CONTEXT_DEVICES,sizeof(cl_device_id)*10,aDevices,&nDevices);
	CL_ERROR("clGetContextInfo");
	nDevices /= sizeof(cl_device_id);
	clBuildProgram(program,nDevices, aDevices,"",0,0);
	// report
	cl_build_status status;
	clGetProgramBuildInfo(program,aDevices[0],CL_PROGRAM_BUILD_STATUS,sizeof(status),&status,0);
	bool bOK = ( status == CL_BUILD_SUCCESS );
	if( bOK )
		printf("OK\n");
	else
		printf("FAIL!\n");
	size_t nLog = 0;
	clGetProgramBuildInfo(program,aDevices[0],CL_PROGRAM_BUILD_LOG,0,0,&nLog);
	if( nLog>0)
	{
		char *text = new char[nLog+1];
		clGetProgramBuildInfo(program,aDevices[0],CL_PROGRAM_BUILD_LOG,nLog+1,text,&nLog);
		printf(text);
		delete [] text;
	}
	delete [] text;
	if( !bOK )
	{
		printf("OpenCL Luce NOT initialized.\n");
		return false;
	}
	// kernel
	kernel = clCreateKernel(program,"Luce",&error);
	CL_ERROR("clCreateKernel");
	printf("OpenCL Luce initialized.\n");		
	return bOK;

#undef CL_ERROR
}

void COpenCLLuce::Do(cl_command_queue queue, Texture* pSrc,Texture* pDst,int lx,int ly)
{
#define CL_ERROR(msg)  if(error != CL_SUCCESS) \
						{ \
							printf("COpenCLLuce::Do ERROR: " msg " fails: %i\n", error); \
							return; \
						}
	cl_int error;

	error = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&(pSrc->GetCL()));
	CL_ERROR("clSetKernelArg 0");
	error = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&(pDst->GetCL()));
	CL_ERROR("clSetKernelArg 1");
	cl_int l[2] = { lx,ly};
	error = clSetKernelArg(kernel, 2, sizeof(cl_int)*2, l );
	CL_ERROR("clSetKernelArg 2");

	cl_mem objs[2] = { pSrc->GetCL(), pDst->GetCL() };
	error = clEnqueueAcquireGLObjects(queue, 2, objs, 0,0,0);
	CL_ERROR("clEnqueueAcquireGLObjects");

	size_t clDim[3] = { 0,1,1 };
	clDim[0] = (pSrc->GetWidth() + pSrc->GetHeight())*2;
	error = clEnqueueNDRangeKernel(queue,kernel,1,NULL,clDim,0,0,0,0);
	CL_ERROR("clEnqueueNDRangeKernel");

	error = clEnqueueReleaseGLObjects(queue,2,objs,0,0,0);
	CL_ERROR("clEnqueueReleaseGLObjects");


#undef CL_ERROR
}
