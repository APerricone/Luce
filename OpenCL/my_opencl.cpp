// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#include "my_opencl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32)
#include <windows.h>
#endif 

#include <GL/glfw.h>

cl_command_queue COpenCL::queue = 0 ;
cl_context COpenCL::context = 0;

void  COpenCL::Init( void )
{
	printf("Initializing openCL...\n");
	cl_platform_id aPlatforms[10];
	cl_uint nPlatform;
	clGetPlatformIDs(10,aPlatforms,&nPlatform);
	if( nPlatform == 0 )
	{
		printf("  FAIL!\nUnable to find an OpenCL platform.");
		exit(-1);
	}
	cl_device_id  aDevices[10];
	cl_uint nDevices;
	cl_device_id  aSelectedDevice;
	cl_uint uiSelectTotal = 0;
	char strSelectName[256];
	unsigned int selP;
	for(unsigned int p=0;p<nPlatform;p++)
	{
		char strText[256];
		clGetPlatformInfo(aPlatforms[p],CL_PLATFORM_NAME,256,strText,0);
		printf("  Check platform: %s", strText);
		clGetPlatformInfo(aPlatforms[p],CL_PLATFORM_VERSION,256,strText,0);
		printf("(%s)\n", strText);

		clGetDeviceIDs(aPlatforms[p], CL_DEVICE_TYPE_ALL, 10,aDevices,&nDevices);
		for(unsigned int d=0;d<nDevices;d++)
		{
			clGetDeviceInfo(aDevices[d],CL_DEVICE_NAME,256,strText,0);
			cl_uint nUnits,nHertz;
			clGetDeviceInfo(aDevices[d],CL_DEVICE_MAX_COMPUTE_UNITS,sizeof(cl_uint),&nUnits,0);
			clGetDeviceInfo(aDevices[d],CL_DEVICE_MAX_CLOCK_FREQUENCY,sizeof(cl_uint),&nHertz,0);			
			printf("    Check device: %s (%u cores at %u Hz)...",strText, nUnits, nHertz);
			size_t iExtensionsSize;
			clGetDeviceInfo(aDevices[d], CL_DEVICE_EXTENSIONS, 0, 0, &iExtensionsSize );
			char* strExtensions = new char[iExtensionsSize];
			clGetDeviceInfo(aDevices[d], CL_DEVICE_EXTENSIONS, iExtensionsSize, strExtensions, &iExtensionsSize);
			if(strstr(strExtensions,"cl_khr_gl_sharing") == 0 )
			{
				delete [] strExtensions;
				printf("REJECTED! no openGl sharing\n");
				continue;
			} else
			/*if(strstr(strExtensions,"cl_khr_3d_image_writes") == 0 )
			{
				delete [] strExtensions;
				printf("REJECTED! no 3d images write\n");
				continue;
			} else*/
			{
				delete [] strExtensions;
				printf("Evaluating\n");
			}

			cl_uint thisTotal = nUnits * nHertz;
			if( thisTotal> uiSelectTotal )
			{
				aSelectedDevice = aDevices[d];
				uiSelectTotal = thisTotal;
				strcpy(strSelectName,strText);
				selP = p;
			}
		}
	}
	if( uiSelectTotal ==  0 )
	{
		printf("FAIL!\nUnable to find an OpenCL platform with OpenGL sharing.");
		exit(-1);
	}
	printf("Selected %s\n", strSelectName );

	#if defined (__APPLE__)
		#warning "this code need verify"
		CGLContextObj kCGLContext = CGLGetCurrentContext();
		CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
		cl_context_properties aProps[] = 
		{
			CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup, 
		0 
		};
		cxGPUContext = clCreateContext(props, 0,0, 0, 0, &ciErrNum);
	#elif defined(UNIX)
		#warning "this code need verify"
		cl_context_properties aProps[] = 
		{
			CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(), 
			CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(), 
			CL_CONTEXT_PLATFORM, (cl_context_properties)aPlatforms[selP], , 
			0
		};
	#elif defined(WIN32)
		cl_context_properties aProps[] = 
		{
			CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(), 
			CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(), 
			CL_CONTEXT_PLATFORM, (cl_context_properties)aPlatforms[selP], 
			0
		};
	#else
		#error "platform not supported"
	#endif

	cl_int error;
	context = clCreateContext(aProps, 1, &aSelectedDevice, 0, 0,&error);
	clError(error);

	CreateCommandQueue();
}

void COpenCL::Deinit()
{
	if(queue) 
	{
		clReleaseCommandQueue(queue); 
		queue=0; 
	}
	if( context )
	{
		clReleaseContext(context);
		context = 0;
	}
}

cl_program COpenCL::CreateProgramFromFile(const char* sFileName,const char* opts)
{
	FILE *f = fopen(sFileName,"rt");
	if( f == 0 )
	{
		printf("CGLSL::LoadShader(): ERROR: Unable to open file \"%s\"",sFileName);
		exit(-1);
	}
	fseek ( f, 0L , SEEK_END ); 
	unsigned int size = ftell(f);
	char *text = new char[size+1];
	fseek ( f, 0L , SEEK_SET ); 
	unsigned int nRead = fread(text,1,size,f);
	text [nRead] = 0;
	fclose(f);
	
	cl_program retValue = CreateProgram(1,(const char**)(&text),opts);
	delete [] text;
	return retValue;
}

cl_program COpenCL::CreateProgram(unsigned int nSource,const char* sources[],const char* opts)
{
	printf("compiling OpenCL program...");
	cl_int error;
	//create
	cl_program retValue = clCreateProgramWithSource(context,nSource,sources,0,&error);
	clError(error);
	// build
	cl_device_id  aDevices[10];
	cl_uint nDevices;
	error = clGetContextInfo(context,CL_CONTEXT_DEVICES,sizeof(cl_device_id)*10,aDevices,&nDevices);
	clError(error);
	nDevices /= sizeof(cl_device_id);
	clBuildProgram(retValue,nDevices, aDevices,opts,0,0);
	// report
	cl_build_status status;
	clGetProgramBuildInfo(retValue,aDevices[0],CL_PROGRAM_BUILD_STATUS,sizeof(status),&status,0);
	bool bExit = ( status != CL_BUILD_SUCCESS );
	if( bExit )
		printf("FAIL!\n");
	else
		printf("OK\n");
	size_t nLog = 0;
	clGetProgramBuildInfo(retValue,aDevices[0],CL_PROGRAM_BUILD_LOG,0,0,&nLog);
	if( nLog>0)
	{
		char *text = new char[nLog+1];
		clGetProgramBuildInfo(retValue,aDevices[0],CL_PROGRAM_BUILD_LOG,nLog+1,text,&nLog);
		printf(text);
		delete [] text;
	}
	
	//exit 
	if( bExit ) 
		exit(-1);
	return retValue;
}

cl_kernel COpenCL::CreateKernel(cl_program oProgram,const char* method)
{
	cl_int error;
	cl_kernel retValue = clCreateKernel(oProgram,method,&error);
	clError(error);
	if( error !=  CL_SUCCESS )
		exit(-1);
	return retValue;
}


void COpenCL::CreateCommandQueue()
{
	cl_int error;
	cl_device_id  aDevices[10];
	cl_uint nDevices;
	error = clGetContextInfo(context,CL_CONTEXT_DEVICES,sizeof(cl_device_id)*10,aDevices,&nDevices);
	clError(error);
	
	queue = clCreateCommandQueue(context,aDevices[0],0,&error);
	clError(error);
}

cl_ulong COpenCL::GetLocalSize()
{
	cl_int error;
	cl_device_id  aDevices[10];
	cl_uint nDevices;
	error = clGetContextInfo(context,CL_CONTEXT_DEVICES,sizeof(cl_device_id)*10,aDevices,&nDevices);
	clError(error);
	
	cl_ulong retValue;
	error = clGetDeviceInfo(aDevices[0], CL_DEVICE_LOCAL_MEM_SIZE,sizeof(cl_ulong),&retValue,0);
	clError(error);
	return retValue;
}

const char* COpenCL::GetErrorString(cl_int err)
{
    switch (err) 
	{
        case CL_SUCCESS:                            return "Success!";
        case CL_DEVICE_NOT_FOUND:                   return "Device not found.";
        case CL_DEVICE_NOT_AVAILABLE:               return "Device not available";
        case CL_COMPILER_NOT_AVAILABLE:             return "Compiler not available";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:      return "Memory object allocation failure";
        case CL_OUT_OF_RESOURCES:                   return "Out of resources";
        case CL_OUT_OF_HOST_MEMORY:                 return "Out of host memory";
        case CL_PROFILING_INFO_NOT_AVAILABLE:       return "Profiling information not available";
        case CL_MEM_COPY_OVERLAP:                   return "Memory copy overlap";
        case CL_IMAGE_FORMAT_MISMATCH:              return "Image format mismatch";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:         return "Image format not supported";
        case CL_BUILD_PROGRAM_FAILURE:              return "Program build failure";
        case CL_MAP_FAILURE:                        return "Map failure";
        case CL_INVALID_VALUE:                      return "Invalid value";
        case CL_INVALID_DEVICE_TYPE:                return "Invalid device type";
        case CL_INVALID_PLATFORM:                   return "Invalid platform";
        case CL_INVALID_DEVICE:                     return "Invalid device";
        case CL_INVALID_CONTEXT:                    return "Invalid context";
        case CL_INVALID_QUEUE_PROPERTIES:           return "Invalid queue properties";
        case CL_INVALID_COMMAND_QUEUE:              return "Invalid command queue";
        case CL_INVALID_HOST_PTR:                   return "Invalid host pointer";
        case CL_INVALID_MEM_OBJECT:                 return "Invalid memory object";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:    return "Invalid image format descriptor";
        case CL_INVALID_IMAGE_SIZE:                 return "Invalid image size";
        case CL_INVALID_SAMPLER:                    return "Invalid sampler";
        case CL_INVALID_BINARY:                     return "Invalid binary";
        case CL_INVALID_BUILD_OPTIONS:              return "Invalid build options";
        case CL_INVALID_PROGRAM:                    return "Invalid program";
        case CL_INVALID_PROGRAM_EXECUTABLE:         return "Invalid program executable";
        case CL_INVALID_KERNEL_NAME:                return "Invalid kernel name";
        case CL_INVALID_KERNEL_DEFINITION:          return "Invalid kernel definition";
        case CL_INVALID_KERNEL:                     return "Invalid kernel";
        case CL_INVALID_ARG_INDEX:                  return "Invalid argument index";
        case CL_INVALID_ARG_VALUE:                  return "Invalid argument value";
        case CL_INVALID_ARG_SIZE:                   return "Invalid argument size";
        case CL_INVALID_KERNEL_ARGS:                return "Invalid kernel arguments";
        case CL_INVALID_WORK_DIMENSION:             return "Invalid work dimension";
        case CL_INVALID_WORK_GROUP_SIZE:            return "Invalid work group size";
        case CL_INVALID_WORK_ITEM_SIZE:             return "Invalid work item size";
        case CL_INVALID_GLOBAL_OFFSET:              return "Invalid global offset";
        case CL_INVALID_EVENT_WAIT_LIST:            return "Invalid event wait list";
        case CL_INVALID_EVENT:                      return "Invalid event";
        case CL_INVALID_OPERATION:                  return "Invalid operation";
        case CL_INVALID_GL_OBJECT:                  return "Invalid OpenGL object";
        case CL_INVALID_BUFFER_SIZE:                return "Invalid buffer size";
        case CL_INVALID_MIP_LEVEL:                  return "Invalid mip-map level";
        default: return "Unknown";
    }
}
