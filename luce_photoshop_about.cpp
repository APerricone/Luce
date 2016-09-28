// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#ifdef STANDALONE
#include <windows.h>
#include "AboutDlg/resource.h"
#else
#include "luce_photoshop.h"
#include <piabout.h>
#include "resource.h"
#endif

#include <stdio.h>
#include <vector>

class CAboutLuce
{
public:
	CAboutLuce();
	~CAboutLuce();
	int Do(HWND hWndParent,HINSTANCE hInstance);
private:
	void ReadBitmap(HINSTANCE hInstance);
	INT_PTR OnInitDialog();
	INT_PTR OnChar(WPARAM wParam);
	INT_PTR OnPaint();
	INT_PTR OnLButtonUp(WPARAM wParam,LPARAM lParam);
	INT_PTR OnMouseMove(WPARAM wParam,LPARAM lParam);
	INT_PTR OnMouseLeave();
	INT_PTR OnSetCursor();

	static INT_PTR CALLBACK DlgFunc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam);

	void Line(long px,long py);
	void Luce();


	struct ThreadLightData
	{
		CAboutLuce *pLuce;
		long startPixel;
		long endPixel; //actually minus+1
		int idx;
		ThreadLightData(CAboutLuce *_pLuce,long _startPixel,long _endPixel,int _idx)
		{
			pLuce = _pLuce;
			startPixel = _startPixel;
			endPixel = _endPixel;
			idx = _idx;
		}
	};
	static DWORD WINAPI ThreadLightFunc(LPVOID ptr);
	std::vector<ThreadLightData> tmp;
	int nThread;

	HWND hDlg;
	bool bDoingLuce;
	RECT siteRect;
	RECT mailRect;
	// The buffers
	long width,height;
	// pixel size: 3 byte
	// stride: width*3 byte
	unsigned char *ptrInput;
	unsigned char *ptrOutput;
	// light position
	int lx,ly;
	unsigned char* lightInput;
	unsigned char* lightOutput;

};

CAboutLuce::CAboutLuce()
{
	ptrInput = NULL;
	ptrOutput = NULL;

	RECT sr = { 193, 286, 375, 301 };
	siteRect = sr;
	RECT mr = { 198, 308, 347, 323 };
	mailRect = mr;
}

CAboutLuce::~CAboutLuce()
{

	free(ptrInput);
	free(ptrOutput);
}

void CAboutLuce::ReadBitmap(HINSTANCE hInstance)
{
	// Open the bitmap
	HBITMAP hBitmap=(HBITMAP)LoadImage(hInstance,
		MAKEINTRESOURCE(IDB_ABOUT),IMAGE_BITMAP,0,0,
		LR_CREATEDIBSECTION);
	// get bits
	BITMAPINFO bmi;
	memset(&bmi,0,sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	HDC hDC=CreateCompatibleDC(0);
	SelectObject(hDC,hBitmap);
	GetDIBits(hDC,hBitmap,0,0,0,&bmi,DIB_RGB_COLORS);
	width=bmi.bmiHeader.biWidth; 
	height=bmi.bmiHeader.biHeight;
	size_t dim=bmi.bmiHeader.biSizeImage;
	if(dim==0) dim=bmi.bmiHeader.biWidth*bmi.bmiHeader.biHeight*bmi.bmiHeader.biBitCount/8;
	unsigned char *tmp=(unsigned char *)(malloc(dim));
	ptrInput=(unsigned char *)(malloc(width*height*3));
	GetDIBits(hDC,hBitmap,0,height,tmp,&bmi,DIB_RGB_COLORS);
	unsigned short x,y;
	unsigned char *s=tmp;
	unsigned char *d=ptrInput;
	for(y=0;y<height;y++)
		for(x=0;x<width;x++)
		{
			d[0]=s[0];
			d[1]=s[1];
			d[2]=s[2];
			s+=bmi.bmiHeader.biBitCount/8;
			d+=3;
		}
	free(tmp);
	DeleteObject(hBitmap);

	ptrOutput=(unsigned char *)(malloc(width*height*3));
}



int CAboutLuce::Do(HWND hDlgParent,HINSTANCE hInstance)
{
	ReadBitmap(hInstance);

	return (DialogBoxParam(hInstance,
		MAKEINTRESOURCE(IDD_ABOUT),(HWND)hDlgParent,DlgFunc,(LPARAM)(this)) == 1);
}

INT_PTR CAboutLuce::OnInitDialog()
{	
    HWND hParent = GetParent(hDlg);
    if  (hParent == NULL) hParent = GetDesktopWindow();

	RECT r;
	GetWindowRect(hParent,&r);
	// The About box should be centered on the main (menu bar) screen, with 1/3 
	// of the remaining space above the dialog, and 2/3 below.
	int xOrigin = (r.right+r.left-width)/2;
	int yOrigin = (r.bottom+r.top-height)/3;
	SetWindowPos(hDlg, NULL, xOrigin, yOrigin, width, height, SWP_NOZORDER);

	lx=width/2;
	ly=height/2;
	// PREPARE MULTI THREAD
	int nPixelToDo = (width+height)*2;
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	nThread = si.dwNumberOfProcessors;
	if( nThread > 1 )
	{
		tmp.reserve( nThread );
		int start = 0;
		for(int i=0;i<nThread;i++)
		{
			int end = MulDiv(nPixelToDo, (i+1), nThread );
			tmp.push_back(ThreadLightData(this,start, end,i) );
			start = end;
		}
	}
	bDoingLuce = false;
	Luce();

	return TRUE;
}

INT_PTR CAboutLuce::OnMouseMove(WPARAM wParam,LPARAM lParam)
{
	if( bDoingLuce ) return TRUE;

	lx=(short)(LOWORD(lParam));
	ly=height-(short)(HIWORD(lParam));
	if( lx<0 || lx>width || 
		ly<0 || ly>height )
	{
		return OnMouseLeave();
	} else
	{
		SetCapture(hDlg);
		OnSetCursor();
	}
	
	Luce();

	return TRUE;
}

INT_PTR CAboutLuce::OnMouseLeave()
{
	if( bDoingLuce ) return TRUE;

	lx=width/2;
	ly=height/2;
	ReleaseCapture();

	Luce();

	return TRUE;
}

INT_PTR CAboutLuce::OnLButtonUp(WPARAM wParam,LPARAM lParam)
{
	POINT p;
	GetCursorPos(&p);
	p.x=(short)(LOWORD(lParam));
	p.y=(short)(HIWORD(lParam));

	if( PtInRect( &siteRect, p) )
	{
		ShellExecute(0,0,"http://amicoperry.altervista.org/luce",0,0,SW_SHOWNORMAL);
		return TRUE;
	}
	if( PtInRect( &mailRect, p) )
	{
		ShellExecute(0,0,"mailto:Antonino Perricone<don_perricone@libero.it>?subject=Luce plugin",0,0,SW_SHOWNORMAL);
		return TRUE;
	}

	EndDialog(hDlg, 1);
	return TRUE;
}

INT_PTR CAboutLuce::OnPaint()
{
	BITMAPINFO bmi;
	memset(&bmi,0,sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth=width;
	bmi.bmiHeader.biHeight=height;
	bmi.bmiHeader.biBitCount=24;
	bmi.bmiHeader.biCompression=BI_RGB;
	bmi.bmiHeader.biPlanes=1;
	HDC hDC=GetDC(hDlg);
	SetDIBitsToDevice(hDC,0,0,width,height,0,0,0,height,ptrOutput,&bmi,DIB_RGB_COLORS);

	ValidateRect(hDlg,0);
	return 1;
}

INT_PTR CAboutLuce::OnSetCursor()
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(hDlg,&p);

	if( PtInRect( &siteRect, p) ||
		PtInRect( &mailRect, p) )
	{
		SetCursor( LoadCursor(NULL,IDC_HAND) );
		return 1;
	}

	SetCursor( LoadCursor(NULL,IDC_ARROW) );
	return 1;
}

INT_PTR CAboutLuce::OnChar(WPARAM wParam)
{
	if( wParam == 13 || // enter
		wParam == 27 ) // esc
	{
		EndDialog(hDlg, 1);
		return TRUE;
	}
	return FALSE;
}

INT_PTR CALLBACK CAboutLuce::DlgFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CAboutLuce *pThis = (CAboutLuce*)(GetWindowLongPtr(hDlg,GWLP_USERDATA));
	if( msg == WM_INITDIALOG)
	{
		pThis = (CAboutLuce*)(lParam);
		SetWindowLongPtr(hDlg,GWLP_USERDATA,(LPARAM)pThis);
	}
	if( pThis == NULL) return  FALSE;
	pThis->hDlg = hDlg;
	switch (msg)
	{
	case WM_INITDIALOG: return pThis->OnInitDialog();
	case WM_CHAR:		return pThis->OnChar(wParam);
	case WM_KILLFOCUS:	return pThis->OnMouseLeave();
	case WM_PAINT:		return pThis->OnPaint();
	case WM_LBUTTONUP:	return pThis->OnLButtonUp(wParam,lParam);
	case WM_MOUSEMOVE:	return pThis->OnMouseMove(wParam,lParam);
	case WM_SETCURSOR:	return pThis->OnSetCursor(); 

	case WM_GETDLGCODE:
		if(lParam)
		{
			LPMSG lpmsg = (LPMSG)lParam;
			if( lpmsg->message == WM_KEYDOWN )
			{
				pThis->OnChar(lpmsg->wParam);
			}
		}
		return DLGC_WANTCHARS;
	}
	return  FALSE;
}

void CAboutLuce::Line(long px,long py)
{
	long x = lx;
	long y = ly;
	long dx=px-lx; //< delta x
	long dy=py-ly; //< delta y
	unsigned long nx=dx<0? -dx :dx; //< number of pixel along x
	unsigned long ny=dy<0? -dy :dy; //< number of pixel along y
	unsigned long np; //< number of pixel along the line
	unsigned char* srcPtr = lightInput;
	unsigned char* dstPtr = lightOutput;
	long xDeltas[2];
	long yDeltas[2];
	ptrdiff_t srcDeltas[2];
	ptrdiff_t dstDeltas[2];
	unsigned long increment;
	int xIdx;
	if(nx>ny)
	{
		np = nx;
		increment = ny;
		xIdx = 0;
	} else
	{
		np = ny;
		increment = nx;
		xIdx = 1;
	}
	xDeltas[1-xIdx] = 0;
	yDeltas[xIdx] = 0;
	if( dx > 0)
	{
		xDeltas[xIdx] = 1;
		srcDeltas[xIdx] = 3;
		dstDeltas[xIdx] = 3;
	} else
	{
		xDeltas[xIdx] = -1;
		srcDeltas[xIdx] = -3;
		dstDeltas[xIdx] = -3;
	}
	if( dy > 0 )
	{
		yDeltas[1-xIdx] = 1;
		srcDeltas[1-xIdx] = width*3;
		dstDeltas[1-xIdx] = width*3;
	} else
	{
		yDeltas[1-xIdx] = -1;
		srcDeltas[1-xIdx] = -width*3;
		dstDeltas[1-xIdx] = -width*3;
	}
	float sum[3]={0.f,0.f,0.f};
	unsigned long fraction=(increment>>1);
	int lightFormState = 0;
	for(unsigned long i=1;i<=np;i++)
	{
		x+=xDeltas[0];
		y+=yDeltas[0];
		fraction+=increment;
		srcPtr+=srcDeltas[0];
		dstPtr+=dstDeltas[0];
		if(fraction>=np)
		{
			x+=xDeltas[1];
			y+=yDeltas[1];
			srcPtr+=srcDeltas[1];
			dstPtr+=dstDeltas[1];
			fraction-=np;
		}
		if( x >= 0 && x < width &&
			y >= 0 && y < height )
		{
			float v[3];
			v[0] = (srcPtr[0])/255.f;
			v[1] = (srcPtr[1])/255.f;
			v[2] = (srcPtr[2])/255.f;
			float n = (float)i;
			if( false ) 
			{
				for(int c=0;c<3;c++)
				{
					sum[c] += v[c];
					v[c] += sum[c]/n;
					if( v[c]>1 ) v[c]=1;
				}
			} else
			{
				for(int c=0;c<3;c++)
				{
					sum[c] += v[c]*(2*n-1);
					v[c] += sum[c]/(n*n);
					if( v[c]>1 ) v[c]=1;
				}
			}
			dstPtr[0] = (unsigned char)(v[0]*255.f);
			dstPtr[1] = (unsigned char)(v[1]*255.f);
			dstPtr[2] = (unsigned char)(v[2]*255.f);
		}
	}
}

DWORD WINAPI CAboutLuce::ThreadLightFunc(LPVOID ptr)
{
	ThreadLightData* pData = (ThreadLightData*)ptr;
	int w = pData->pLuce->width;
	int h = pData->pLuce->height;
	//while(false)
	{
		long idx = 0;
		for(int x=0;x<w;x++)
		{
			if( idx >= pData->startPixel && idx < pData->endPixel )
				pData->pLuce->Line(x,0);
			idx++;
		}
		for(int y=1;y<h;y++)
		{
			if( idx >= pData->startPixel && idx < pData->endPixel )
				pData->pLuce->Line(w-1,y);
			idx++;
		}
		for(int x=w-2;x>=0;x--)
		{
			if( idx >= pData->startPixel && idx < pData->endPixel )
				pData->pLuce->Line(x,h-1);
			idx++;
		}
		for(int y=h-1;y>0;y--)
		{
			if( idx >= pData->startPixel && idx < pData->endPixel )
				pData->pLuce->Line(0,y);
			idx++;
		}
	}
	return 1;
}

//#define SHOWPERFORMANCE
void CAboutLuce::Luce()
{
	if( bDoingLuce ) return;
	bDoingLuce = true;
#ifdef SHOWPERFORMANCE
	LARGE_INTEGER frequency,startValue,current;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&startValue);
#endif //SHOWPERFORMANCE
	lightInput = &(ptrInput[(lx + ly * width)*3]);
	lightOutput = &(ptrOutput[(lx + ly * width)*3]);
	if( lx >= 0 && lx < width &&
		ly >= 0 && ly < height )
	{
		*((long*)lightOutput) = *((long*)lightInput);
	}
	if(nThread>1)
	{
		std::vector<HANDLE> threads;
		threads.resize(nThread);
		for(int i=0;i<nThread;i++)
		{
			threads[i] = CreateThread(0,0,ThreadLightFunc,&tmp[i],0,0);
		}
		WaitForMultipleObjects(nThread,&threads[0],TRUE,INFINITE);
	} else
	{
		ThreadLightFunc(&tmp[0]);
	}

	bDoingLuce = false;
#ifdef SHOWPERFORMANCE
	char buff[100];
    QueryPerformanceCounter(&current);
	sprintf(buff,"Luce duration: %f\n", 
		(double)(current.QuadPart - startValue.QuadPart) / (double)frequency.QuadPart);
	OutputDebugStr(buff);
#endif //SHOWPERFORMANCE
	OnPaint();
}

#ifdef STANDALONE
int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE,LPSTR,int)
{
	CAboutLuce dlg;
	return dlg.Do(NULL,hInstance);
}
#else
void CPhotoshopLuce::DoAbout()
{
	AboutRecord* pAboutRecord = (AboutRecord*)pFilterRecord;
	PlatformData *pPlatform = (PlatformData *) (pAboutRecord->platformData);	
	HWND hWnd = (HWND)pPlatform ->hwnd;

	CAboutLuce dlg;

	dlg.Do((HWND)hWnd,hInstance);
}
#endif