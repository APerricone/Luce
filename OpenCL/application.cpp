// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#include "application.h"

#include "gflTexture.h"
#include "openCLLuce.h"

#include <stdio.h>

CApplication::CApplication()
{
	pSource = NULL;
	pDest = NULL;
	pLuce = NULL;
	bDoingLuce = true;
}

CApplication::~CApplication()
{
	if( pSource ) delete pSource;
	if( pDest ) delete pDest;
	if( pLuce ) delete pLuce;
	gflTexture::DeinitLib();
}

int CApplication::Do(int argc, char **argv)
{
	InitDebug();
	bool bTryDefault = true;
	if( argc>1 )
	{
		if( LoadSource(argv[1]) )
			bTryDefault = false;
	}
	if( bTryDefault )
	{
		if( !LoadSource(NULL) ) return -1;
	}
	if( !InitWindow() ) return -2;
	if( !InitOpenGL() ) return -3;
	if( !InitOpenCL() ) return -4;
	if( !PostInit() ) return -5;

	OnUpdate();
	return DoLoop();
}

bool CApplication::LoadSource(const char* fileName)
{
	gflTexture::InitLib();

	pSource = new gflTexture();
	if( fileName == NULL || fileName[0] == 0 ) 
		fileName = "source.png";
	if( !pSource->Create(fileName) )
		return false;
	return true;
}

bool CApplication::InitOpenCL()
{
	printf("Init OpenCL...");
	cl_platform_id aPlatforms[10];
	cl_uint nPlatform;
	clGetPlatformIDs(10,aPlatforms,&nPlatform);
	if( nPlatform == 0 )
	{
		printf("ERROR: can't init OpenGL.(clGetPlatformIDs fails)\n");
		return false;
	}
	printf("\n",nPlatform);
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

		clGetDeviceIDs(aPlatforms[p], CL_DEVICE_TYPE_GPU, 10,aDevices,&nDevices);
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
				printf("REJECTED: no openGl sharing\n");
				continue;
			} else
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
		printf("ERROR: Unable to find an OpenCL platform with OpenGL sharing.\n");
		return false;
	}
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
	if(error != CL_SUCCESS) 
	{
		printf("ERROR: can't init OpenGL.(clCreateContext fails:%i)\n", error);
		return false;
	}

	//CreateCommand
	error = clGetContextInfo(context,CL_CONTEXT_DEVICES,sizeof(cl_device_id)*10,aDevices,&nDevices);
	if(error != CL_SUCCESS) 
	{
		printf("ERROR: can't init OpenGL.(clGetContextInfo fails:%i)\n", error);
		return false;
	}

	queue = clCreateCommandQueue(context,aDevices[0],0,&error);
	if(error != CL_SUCCESS) 
	{
		printf("ERROR: can't init OpenGL.(clCreateCommandQueue fails:%i)\n", error);
		return false;
	}
	printf("OK: Selected %s\n", strSelectName );
	return true;
}

void CApplication::OnMouseMove(int x,int y)
{
	lx = x;
	ly = pSource->GetHeight() - y;
	if( lx < 0 ) lx=0;
	if( lx >= pSource->GetWidth() ) lx = pSource->GetWidth()-1;
	if( ly < 0 ) ly=0;
	if( ly >= pSource->GetHeight() ) ly = pSource->GetHeight()-1;
	OnUpdate();
}

#define SHOWPERFORMANCE
void CApplication::OnUpdate()
{
	if( bDoingLuce ) return;

	bDoingLuce = true;
#ifdef SHOWPERFORMANCE
	LARGE_INTEGER frequency,startValue,current;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&startValue);
#endif //SHOWPERFORMANCE
	pLuce->Do(queue,pSource,pDest, lx,ly);
	// Wait for OpenCL - This program does only this :)
	/*cl_int error;
	error = clFinish(queue);
	if( error != CL_SUCCESS )
	{
		printf("ERROR clFinish: %i\n", error );
	}
	//*/
	OnPaint();
#ifdef SHOWPERFORMANCE
    QueryPerformanceCounter(&current);
	char buff[100];
	sprintf(buff,"RealTimeLuce: %f ms\r", 1000. *
		(double)(current.QuadPart - startValue.QuadPart) /
		(double)frequency.QuadPart);	
	SetWindowText(hWnd,buff);
#endif //SHOWPERFORMANCE

	bDoingLuce = false;
}

void CApplication::OnPaint()
{
	glClearColor(0,0,0,255);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	pDest->SetIt();
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0,0); glVertex2f(-1,-1);
	glTexCoord2f(1,0); glVertex2f( 1,-1);
	glTexCoord2f(0,1); glVertex2f(-1, 1);
	glTexCoord2f(1,1); glVertex2f( 1, 1);
	glEnd();

#if defined(WIN32)
	SwapBuffers(hDC);
#else
	#error "platform not supported"
#endif
}

bool CApplication::PostInit()
{
	// OpenGL
	if( !pSource->Init() ) return false;
	pDest = new Texture();
	if( !pDest->Create(pSource->GetWidth(),pSource->GetHeight(),pSource->GetBPP()) ) return false;
	if( !pDest->Init() ) return false;
	// OpenCL
	if( !pSource->InitCL(context,CL_MEM_READ_ONLY) ) return false;
	if( !pDest->InitCL(context,CL_MEM_WRITE_ONLY) ) return false;
	// Luce
	pLuce = new COpenCLLuce();
	if( !pLuce->Init(context) ) return false;
	lx = pSource->GetWidth()/2;
	ly = pSource->GetHeight()/2;
	bDoingLuce = false;
	return true;
}
