// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#include "DialControl.h"
#include <windowsx.h>
#define _USE_MATH_DEFINES
#include <math.h>

LRESULT CALLBACK DialWndProc( HWND, UINT, WPARAM, LPARAM);

BOOL RegisterDialControl(HINSTANCE hInstance)
{
    WNDCLASS wndcls;
#ifdef _DEBUG
	UnregisterClass(DIALCTRLCLASSNAME,hInstance);
#endif 
    if (!(GetClassInfo(hInstance, DIALCTRLCLASSNAME, &wndcls)))
    {	// register the class
        wndcls.style            = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
        wndcls.lpfnWndProc      = DialWndProc;
        wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
        wndcls.hInstance        = hInstance;
        wndcls.hIcon            = NULL;
		wndcls.hCursor          = LoadCursor(NULL,IDC_ARROW);
        wndcls.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
        wndcls.lpszMenuName     = NULL;
        wndcls.lpszClassName    = DIALCTRLCLASSNAME;

        if (!RegisterClass(&wndcls))
        {
            return FALSE;
        }
    }

    return TRUE;
}

void UnregisterDialControl(HINSTANCE hInstance)
{
	UnregisterClass(DIALCTRLCLASSNAME,hInstance);
}


//! container for internal data
typedef struct 
{
	//! value actual saved inside the dial control
	double value;
	//! minimum value can be set
	double min;
	//! maximum value can be set
	double max;
	//! step, delta angle to apply when the use press right,left up or down
	double step;
	//! page, delta angle to apply when the use press pgup, or pgdown
	double page;
	//!
	char useMinMax;
} DialInternalData;

void DialInternalDataInit(DialInternalData* pDest)
{
	pDest->value = 0.;
	pDest->min = -M_PI;
	pDest->max = M_PI;
	pDest->step = M_PI / 180;
	pDest->page = M_PI / 18;
	pDest->useMinMax = 0;
}

//! useful method to lock the data
DialInternalData* DialLockData(HWND hWnd)
{
	HLOCAL hLocal;
	DialInternalData* r;
	hLocal = ((HLOCAL)(GetWindowLongPtr(hWnd,GWLP_USERDATA)));
	r = ((DialInternalData*)(LocalLock(hLocal)));
	if( r== NULL)
	{
		DWORD e = GetLastError();
		e=e;
	}
	return r;
}

//! useful method to unlock the data
BOOL DialUnlockData(HWND hWnd)
{
	HLOCAL hLocal;
	hLocal = ((HLOCAL)(GetWindowLongPtr(hWnd,GWLP_USERDATA)));
	return LocalUnlock(hLocal);
}

//! paint manageing
LRESULT DialPaint(HWND hWnd)
{
	// rectangle area of control
	RECT r;		
	// center and radius
	LONG centerx,centery,radius,r10;
	// direction of line
	LONG dx,dy;
	// Windows stuff
	PAINTSTRUCT ps;
	HDC hDC;
	HPEN oldPen, pen;
	HBRUSH oldBrush, brush;
	// control's main color
	DWORD color;
	//
	GetClientRect(hWnd,&r);
	// calculate center and radius
	centerx = r.right/2;
	centery = r.bottom/2;
	if( r.right < r.bottom )
	{
		radius = r.right / 2;
		r.top = centery - radius;
		r.bottom = centery + radius;
	} else
	{
		radius = r.bottom / 2;
		r.left = centerx - radius;
		r.right = centerx + radius;
	}
	r10 = radius/10;
	if( r10 < 1 ) r10 = 1;

	hDC = BeginPaint(hWnd,&ps);
	SelectObject(hDC,GetStockObject(HOLLOW_BRUSH));
	
	color = 0;
	if( !IsWindowEnabled(hWnd) ) 
		color = GetSysColor(COLOR_GRAYTEXT);
	pen = CreatePen(PS_SOLID,1,color);

	oldPen = (HPEN)SelectObject(hDC,pen);
	Ellipse(hDC,r.left,r.top,r.right,r.bottom);
	Ellipse(hDC,r.left+4,r.top+4,r.right-4,r.bottom-4);

	brush = CreateSolidBrush(color);
	oldBrush = (HBRUSH)SelectObject(hDC,brush);

	Ellipse(hDC,centerx-r10*2,centery-r10*2,
				centerx+r10*2,centery+r10*2);

	SelectObject(hDC,oldPen);
	SelectObject(hDC,oldBrush);
	DeleteObject(pen);
	DeleteObject(brush);

	if( IsWindowEnabled(hWnd) ) 
	{
		// draw angle line
		DialInternalData* pData = DialLockData(hWnd);

		pen = CreatePen(PS_SOLID,r10,color);
		oldPen = (HPEN)SelectObject(hDC,pen);
		dx = (LONG)(cos(pData->value) * radius);
		dy = (LONG)(-sin(pData->value) * radius);
		MoveToEx(hDC,centerx,centery,NULL);
		LineTo(hDC,centerx+dx,centery+dy);
		SelectObject(hDC,oldPen);
		DeleteObject(pen);

		DialUnlockData(hWnd);
	}

	if(GetFocus() == hWnd)
	{	
		// draw focus indicator
		pen = CreatePen(PS_DOT,1,RGB(0,0,0));
		oldPen = (HPEN)SelectObject(hDC,pen);
		Ellipse(hDC,r.left+2,r.top+2,r.right-2,r.bottom-2);
		SelectObject(hDC,oldPen);
		DeleteObject(pen);
	}

	EndPaint(hWnd,&ps);
	return 0;
}

LRESULT DialSetValue(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// source fo angle
	double *src = (double*)lParam;
	// Internal data
	DialInternalData* pData;
	// Parent window
	HWND hParent;
	//
	if( src == NULL) return FALSE;
	//
	pData = DialLockData(hWnd);
	pData->value = *src;
	if( pData->useMinMax )
	{
		if( pData->value < pData->min ) pData->value = pData->min;
		if( pData->value > pData->max ) pData->value = pData->max;
	}
	DialUnlockData(hWnd);
	if( wParam != FALSE )
		InvalidateRect(hWnd,NULL,TRUE);

	hParent = GetParent(hWnd);
	if( hParent )
	{
		SendMessage(hParent,WM_COMMAND,
			MAKELONG(GetDlgCtrlID(hWnd),DCN_CHANGING),
			(LPARAM)(hWnd));
		SendMessage(hParent,WM_COMMAND,
			MAKELONG(GetDlgCtrlID(hWnd),DCN_CHANGED),
			(LPARAM)(hWnd));
	}

	return TRUE;
}

LRESULT DialSetStep(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// source
	double *src = (double*)lParam;
	// internal data
	DialInternalData* pData;
	if( src == NULL) return FALSE;
	pData = DialLockData(hWnd);
	pData->step = *src;
	DialUnlockData(hWnd);
	if( wParam != FALSE )
		InvalidateRect(hWnd,NULL,TRUE);
	return TRUE;
}

LRESULT DialSetPage(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// source
	double *src = (double*)lParam;
	// internal data
	DialInternalData* pData;
	if( src == NULL) return FALSE;
	pData = DialLockData(hWnd);
	pData->step = *src;
	DialUnlockData(hWnd);
	if( wParam != FALSE )
		InvalidateRect(hWnd,NULL,TRUE);
	return TRUE;
}

LRESULT DialGetValue(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// destination
	double *dest = (double*)lParam;
	// internal data
	DialInternalData* pData;
	if( dest == NULL) return FALSE;
	pData = DialLockData(hWnd);
	*dest = pData->value;
	DialUnlockData(hWnd);
	return TRUE;
}

LRESULT DialMouseMove( HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if( wParam & MK_LBUTTON )
	{
		// rectangle area of control
		RECT r;		
		// center and radius
		LONG centerx,centery;
		// mouse position
		int x,y;
		// internal data
		DialInternalData* local;
		// parent window
		HWND hParent;
		
		GetClientRect(hWnd,&r);
		centerx = r.right/2;
		centery = r.bottom/2;
		x = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);
		local=DialLockData(hWnd);
		local->value = atan2( (double)(centery-y), (double)(x-centerx) );
		DialUnlockData(hWnd);
		InvalidateRect(hWnd,NULL,TRUE);
		hParent = GetParent(hWnd);
		if( hParent )
		{
			SendMessage(hParent,WM_COMMAND,
				MAKELONG(GetDlgCtrlID(hWnd),DCN_CHANGING),
				(LPARAM)(hWnd));
		}
	}
	return 0;
}

LRESULT DialLButtonDown( HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	SetFocus(hWnd);
	SetCapture(hWnd);
	return DialMouseMove(hWnd,wParam,lParam);
}

LRESULT DialLButtonUp( HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// parent window
	HWND hParent = GetParent(hWnd);
	ReleaseCapture();
	if( hParent )
	{
		SendMessage(hParent,WM_COMMAND,
			MAKELONG(GetDlgCtrlID(hWnd),DCN_CHANGING),
			(LPARAM)(hWnd));
		SendMessage(hParent,WM_COMMAND,
			MAKELONG(GetDlgCtrlID(hWnd),DCN_CHANGED),
			(LPARAM)(hWnd));
	}
	return 0;
}

LRESULT DialKeyDown( HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	BOOL bNotify = FALSE;
	DialInternalData* pData = NULL;
	if( wParam == VK_PRIOR )
	{
		pData = DialLockData(hWnd);
		pData->value += pData->page;
		bNotify = TRUE;
		DialUnlockData(hWnd);
	}
	if( wParam == VK_NEXT )
	{
		pData = DialLockData(hWnd);
		pData->value -= pData->page;
		bNotify = TRUE;
		DialUnlockData(hWnd);
	}
	if( wParam == VK_UP || wParam == VK_LEFT )
	{
		pData = DialLockData(hWnd);
		pData->value += pData->step;
		bNotify = TRUE;
		DialUnlockData(hWnd);
	}
	if( wParam == VK_DOWN || wParam == VK_RIGHT )
	{
		pData = DialLockData(hWnd);
		pData->value -= pData->step;
		bNotify = TRUE;
		DialUnlockData(hWnd);
	}
	
	if( bNotify )
	{
		HWND hParent = GetParent(hWnd);

		if( pData->useMinMax )
		{
			if( pData->value < pData->min ) pData->value = pData->min;
			if( pData->value > pData->max ) pData->value = pData->max;
		}
		DialUnlockData(hWnd);

		InvalidateRect(hWnd,NULL,TRUE);
		if( hParent )
		{
			SendMessage(hParent,WM_COMMAND,
				MAKELONG(GetDlgCtrlID(hWnd),DCN_CHANGING),
				(LPARAM)(hWnd));
			SendMessage(hParent,WM_COMMAND,
				MAKELONG(GetDlgCtrlID(hWnd),DCN_CHANGED),
				(LPARAM)(hWnd));
		}
	}
	return 0;
}

LRESULT CALLBACK DialWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_CREATE:
		SetWindowLongPtr(hWnd,GWLP_USERDATA,
			(LONG_PTR)(LocalAlloc(LMEM_FIXED,sizeof(DialInternalData))));
		{
			DialInternalData* pData = DialLockData(hWnd);
			DialInternalDataInit(pData);
			DialUnlockData(hWnd);
		}
		break;
	case WM_DESTROY:
		LocalFree((HLOCAL)(GetWindowLongPtr(hWnd,GWLP_USERDATA)));
		break;
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		InvalidateRect(hWnd,NULL,TRUE);
		break;
	case WM_PAINT:
		return DialPaint(hWnd);
	case WM_LBUTTONDOWN:
		return DialLButtonDown(hWnd,wParam,lParam);
	case WM_LBUTTONUP:
		return DialLButtonUp(hWnd,wParam,lParam);
	case WM_MOUSEMOVE:
		return DialMouseMove(hWnd,wParam,lParam);
	case WM_KEYDOWN:
		return DialKeyDown(hWnd,wParam,lParam);
	case WM_GETDLGCODE:
		return DLGC_WANTARROWS;
	// MESSAGES
	case DCM_SETVALUE:
		return DialSetValue(hWnd,wParam,lParam);
	case DCM_SETSTEP:
		return DialSetStep(hWnd,wParam,lParam);
	case DCM_SETPAGE:
		return DialSetPage(hWnd,wParam,lParam);
	case DCM_GETVALUE:
		return DialGetValue(hWnd,wParam,lParam);

	}
	return DefWindowProc (hWnd,msg,wParam,lParam);
}

