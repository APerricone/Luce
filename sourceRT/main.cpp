// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#include <windows.h>
#include <stdio.h>
#include <vector>
#include <xmmintrin.h>
#include <mmintrin.h>

#include "repetive_thread.h"

class CRealTimeLuce
{
public:
	CRealTimeLuce();
	~CRealTimeLuce();
	int Do(HINSTANCE hInstance);
private:
	bool ReadBitmap(HINSTANCE hInstance);
	INT_PTR OnPaint();
	INT_PTR OnMouseMove(WPARAM wParam,LPARAM lParam);
	bool close;
	static LRESULT CALLBACK WndFunc(HWND,UINT,WPARAM,LPARAM);

	void Line(long px,long py);
	void Luce();

	struct ThreadLightData
	{
		CRealTimeLuce *pLuce;
		long startPixel;
		long endPixel; //actually minus+1
		int idx;
		ThreadLightData(CRealTimeLuce *_pLuce,long _startPixel,long _endPixel,int _idx)
		{
			pLuce = _pLuce;
			startPixel = _startPixel;
			endPixel = _endPixel;
			idx = _idx;
		}
	};
	static void ThreadLightFunc(void* ptr);
	static DWORD WINAPI ThreadFunc2(LPVOID ptr);
	std::vector<ThreadLightData> tmp;
	std::vector<CRepetitiveThread*> threads;
	int nThread;


	HWND hWnd;
	bool bDoingLuce;
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

CRealTimeLuce::CRealTimeLuce()
{
	ptrInput = NULL;
	ptrOutput = NULL;
}

CRealTimeLuce::~CRealTimeLuce()
{
	for(int i=0;i<nThread;i++)
	{
		delete threads[i];
	}
	free(ptrInput);
	free(ptrOutput);
}

bool CRealTimeLuce::ReadBitmap(HINSTANCE hInstance)
{
	// Open the bitmap
	HBITMAP hBitmap=(HBITMAP)LoadImage(hInstance,"source.bmp",IMAGE_BITMAP,0,0,
		LR_LOADFROMFILE|LR_CREATEDIBSECTION);
	if(hBitmap==0)
	{
		MessageBox(0,"The file source.bmp don't found,\n the application will be closed.",
			"Error",MB_OK | MB_ICONERROR);
		return false;
	}
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
	return true;
}

int CRealTimeLuce::Do(HINSTANCE hInstance)
{
	close = false;
	if( !ReadBitmap(hInstance) )
		return -1;

	WNDCLASS myClass={CS_DBLCLKS,WndFunc,0,0,hInstance,NULL,
		LoadCursor(NULL,IDC_ARROW),(HBRUSH)GetStockObject(BLACK_BRUSH),NULL,"RealTimeLuce"};
	RegisterClass(&myClass);
#define WINDOWSTILE WS_SYSMENU|WS_BORDER|WS_CAPTION
	RECT r;
	r.right=GetSystemMetrics(SM_CXSCREEN);
	r.bottom=GetSystemMetrics(SM_CYSCREEN);
	r.left=(r.right-width)/2;
	r.top=(r.bottom-height)/2;
	r.right=r.left+width;
	r.bottom=r.top+height;
	AdjustWindowRect(&r,WINDOWSTILE,FALSE);
	hWnd=CreateWindow("RealTimeLuce","RealTimeLuce",WINDOWSTILE,
		r.left,r.top,r.right-r.left,r.bottom-r.top,NULL,NULL,hInstance,this);
	ShowWindow(hWnd,SW_SHOW);
	UpdateWindow(hWnd);
	// Init Luce
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
		threads.reserve( nThread );
		int start = 0;
		for(int i=0;i<nThread;i++)
		{
			int end = MulDiv(nPixelToDo, (i+1), nThread );
			tmp.push_back(ThreadLightData(this,start, end,i) );
			start = end;
		}
		for(int i=0;i<nThread;i++)
		{
			threads.push_back(new CRepetitiveThread(ThreadLightFunc,&tmp[i]) );
		}
	}
	bDoingLuce = false;

	MSG msg;
	while(	!close && GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	};
	return 0;
}

INT_PTR CRealTimeLuce::OnMouseMove(WPARAM wParam,LPARAM lParam)
{
	if( bDoingLuce ) return TRUE;

	lx=(short)(LOWORD(lParam));
	ly=height-(short)(HIWORD(lParam));
	if( lx<0 ) lx = 0;
	if( ly<0 ) ly = 0;
	if( lx>=width )lx = width-1;
	if( ly>=height )ly = height-1;
	
	Luce();

	return TRUE;
}

INT_PTR CRealTimeLuce::OnPaint()
{
	BITMAPINFO bmi;
	memset(&bmi,0,sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth=width;
	bmi.bmiHeader.biHeight=height;
	bmi.bmiHeader.biBitCount=24;
	bmi.bmiHeader.biCompression=BI_RGB;
	bmi.bmiHeader.biPlanes=1;
	HDC hDC=GetDC(hWnd);
	SetDIBitsToDevice(hDC,0,0,width,height,0,0,0,height,ptrOutput,&bmi,DIB_RGB_COLORS);

	ValidateRect(hWnd,0);
	return 1;
}

LRESULT CALLBACK CRealTimeLuce::WndFunc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CRealTimeLuce *pThis = (CRealTimeLuce*)(GetWindowLongPtr(hWnd,GWLP_USERDATA));
	if( msg == WM_CREATE )
	{
		CREATESTRUCT* pCreate = (CREATESTRUCT*)(lParam);
		pThis = (CRealTimeLuce*)(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd,GWLP_USERDATA,(LPARAM)pThis);
	}
	if( pThis == NULL )
		return DefWindowProc (hWnd,msg,wParam,lParam);
	pThis->hWnd = hWnd;
	switch (msg)
	{
	case WM_PAINT:		return pThis->OnPaint();
	case WM_MOUSEMOVE:	return pThis->OnMouseMove(wParam,lParam);
	case WM_DESTROY:
		pThis->close=true;
		return 0;
	}
	return DefWindowProc (hWnd,msg,wParam,lParam);
}

void CRealTimeLuce::Line(long px,long py)
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

void CRealTimeLuce::ThreadLightFunc(void* ptr)
{
	ThreadLightData* pData = (ThreadLightData*)ptr;
	if( pData == NULL ) return;
	int w = pData->pLuce->width;
	int h = pData->pLuce->height;
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

DWORD WINAPI CRealTimeLuce::ThreadFunc2(LPVOID ptr)
{
	ThreadLightFunc(ptr);
	return 0;
}

#define SHOWPERFORMANCE
void CRealTimeLuce::Luce()
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
		for(int i=0;i<nThread;i++)
		{
			threads[i]->Start();
		}
		for(int i=0;i<nThread;i++)
		{
			threads[i]->WaitEnd();
		}
	} else
	{
		ThreadLightFunc(&tmp[0]);
	}

	bDoingLuce = false;
#ifdef SHOWPERFORMANCE
	char buff[100];
    QueryPerformanceCounter(&current);
	sprintf(buff,"RealTimeLuce: %f fps\n", 
		(double)frequency.QuadPart /
		(double)(current.QuadPart - startValue.QuadPart));
	SetWindowText(hWnd,buff);
#endif //SHOWPERFORMANCE
	OnPaint();
}

// WinMain
int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE,LPSTR,int)
{
	CRealTimeLuce dlg;
	return dlg.Do(hInstance);
}
