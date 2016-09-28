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

#define SAFE_DELETE_CL_PROGRAM(a) { if(a) clReleaseProgram(a); a=0; }
#define SAFE_DELETE_CL_KERNEL(a) { if(a) clReleaseKernel(a); a=0; }
#define SAFE_DELETE_CL_MEMOBJ(a) { if(a) clReleaseMemObject(a); a=0; }

#if _DEBUG
#define clError(err) { \
	if(err != CL_SUCCESS) { \
		fprintf(stderr, "%s(%i):ERROR: OpenCL \"%s\"\n", __FILE__, __LINE__, (char *)COpenCL::GetErrorString(err)); \
	} \
}
#else
#define clError(err) (err)
#endif

class COpenCL
{
public:
	static void Init();
	static void Deinit();

	static cl_program CreateProgramFromFile(const char* sFileName,const char* opts);
	static cl_program CreateProgram(unsigned int nSource,const char* sources[],const char* opts);

	static cl_kernel CreateKernel(cl_program oProgram,const char* method);

	static const char* GetErrorString(cl_int err);

	static  cl_ulong GetLocalSize(); 

	static cl_context context;
	static cl_command_queue queue;
private:
	static void CreateCommandQueue();
};
