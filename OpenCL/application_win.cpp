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
#include <stdio.h>
#include <cstring>
//#ifdef _DEBUG
#include <io.h>
#include <fcntl.h>
//#endif //_DEBUG

#include "gflTexture.h"

int APIENTRY WinMain(HINSTANCE	hInstance,HINSTANCE,LPSTR,int)
{
	char dr[100];
	GetCurrentDirectory(100,dr);
	CApplication tmp;
	tmp.hInstance = hInstance;

	return tmp.Do(__argc,__argv);
}

#define WINDOWSNAME "Luce OpenCL"

int CApplication::DoLoop()
{
	MSG message;
	while( GetMessage (&message,NULL,0,0) )
	{
		OnPaint();
		TranslateMessage (&message);
		DispatchMessage (&message);
	}
	return message.wParam;
}

void CApplication::InitDebug()
{
//#ifdef _DEBUG
	AllocConsole();
	AttachConsole(ATTACH_PARENT_PROCESS);
	SetConsoleTitle(WINDOWSNAME" - log");

	int hCrt, i;
	FILE *hf;
	hCrt = _open_osfhandle((long) GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
	hf = _fdopen( hCrt, "w" );
	*stdout = *hf;
	i = setvbuf( stdout, NULL, _IONBF, 0 ); 
//#endif
	char buff[MAX_PATH];
	GetModuleFileName(NULL,buff,MAX_PATH);
	char *pSlash =strrchr(buff,'\\');
	if( pSlash )
	{
		*pSlash = 0;
		SetCurrentDirectory( buff );
	}
}

bool CApplication::InitWindow()
{
	printf("Init window...");
	WNDCLASS MyClasse={CS_DBLCLKS,WndProc,0,0,hInstance,NULL,
		LoadCursor(NULL,IDC_ARROW),(HBRUSH)GetStockObject(BLACK_BRUSH),NULL,WINDOWSNAME};
	RegisterClass( &MyClasse);
#define WINDOWSTILE WS_SYSMENU|WS_BORDER|WS_CAPTION
	RECT r; int cx,cy;
	r.right=GetSystemMetrics(SM_CXSCREEN);
	r.bottom=GetSystemMetrics(SM_CYSCREEN);
	cx=r.left=r.right/2;
	cy=r.top=r.bottom/2;
	r.right = pSource->GetWidth();
	r.bottom = pSource->GetHeight();
	r.left-=r.right/2;
	r.right+=r.left;
	r.top-=r.bottom/2;
	r.bottom+=r.top;
	AdjustWindowRect(&r,WINDOWSTILE,FALSE);
	hWnd=
		CreateWindow (WINDOWSNAME,WINDOWSNAME,WINDOWSTILE,
		r.left,r.top,r.right-r.left,r.bottom-r.top,NULL,NULL,hInstance,this);
	ShowWindow(hWnd,SW_SHOW);
	UpdateWindow (hWnd);
	printf("OK\n");
	return true;
}

bool CApplication::InitOpenGL()
{
	printf("Init OpenGL...");
	hDC=GetDC(hWnd);
	PIXELFORMATDESCRIPTOR pfd ;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR)) ;
	pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion   = 1 ;
	pfd.dwFlags    = PFD_DOUBLEBUFFER |
					PFD_SUPPORT_OPENGL |
					PFD_DRAW_TO_WINDOW ;
	pfd.cColorBits = 32;
	pfd.iPixelType = PFD_TYPE_RGBA ;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE ;
	int nPixelFormat = ChoosePixelFormat(hDC, &pfd);
	if(nPixelFormat == 0)
	{
		ReleaseDC(hWnd,hDC);
		printf("ERROR: can't init OpenGL.(ChoosePixelFormat fails)\n");
		return false;
	}
	/*BOOL bResult = */SetPixelFormat(hDC, nPixelFormat, &pfd);
	HGLRC m_hrc = wglCreateContext(hDC);
	if (!m_hrc)
	{
		ReleaseDC(hWnd,hDC);
		printf("ERROR: can't init OpenGL.(wglCreateContext fails)\n");
		return false;
	}
	wglMakeCurrent (hDC, m_hrc);
	printf("OK\n");
	// test
	typedef BOOL (APIENTRY * PFNWGLSWAPINTERVALEXTPROC) (int interval);
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	if( wglSwapIntervalEXT )
		wglSwapIntervalEXT(0);
	//
	return true;
}


LRESULT CALLBACK CApplication::WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CApplication *pApp = (CApplication*)(GetWindowLongPtr(hWnd,GWLP_USERDATA));
	if( msg == WM_CREATE )
	{
		CREATESTRUCT* pCreate = (CREATESTRUCT*)(lParam);
		pApp = (CApplication*)(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd,GWLP_USERDATA,(LPARAM)pApp);
	}
	if( pApp == NULL )
		return DefWindowProc (hWnd,msg,wParam,lParam);
	switch (msg)
	{
	case WM_MOUSEMOVE:
		pApp->OnMouseMove(LOWORD(lParam), HIWORD(lParam));
		return 1;
	case WM_PAINT:
		pApp->OnPaint();
		ValidateRect(hWnd,NULL);
		return 1;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc (hWnd,msg,wParam,lParam);
}