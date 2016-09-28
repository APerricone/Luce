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

#ifdef _WINDOWS
#include <Windows.h>
#endif

#include <gl/GL.h>
#include <CL/opencl.h>

class Texture;
class gflTexture;
class COpenCLLuce;

class CApplication
{
public:
#ifdef _WINDOWS
	HINSTANCE hInstance;
#endif

	CApplication();
	~CApplication();

	int Do(int argc, char **argv);

private:
	void InitDebug();
	bool LoadSource(const char* fileName);
	bool InitWindow();
	bool InitOpenGL();
	bool InitOpenCL();
	bool PostInit();

	int DoLoop();

	//callbacks
	void OnUpdate();
	void OnMouseMove(int x,int y);
	void OnPaint();

#ifdef _WINDOWS
	HWND hWnd;
	HDC hDC;
	static LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM);
#endif

	cl_context context;
	cl_command_queue queue;

	gflTexture *pSource;
	Texture *pDest;

	COpenCLLuce *pLuce;
	int lx,ly;

	bool bDoingLuce;
};
