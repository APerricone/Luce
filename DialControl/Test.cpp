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
#include "resource.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

//! use common controls 6.0
#pragma comment(linker, \
	"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls'" \
	"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"") 

#include "DialControl.h"

BOOL CALLBACK MainProc(HWND hDlg,UINT MSG,WPARAM wPar,LPARAM lPar)
{
	switch (MSG)
	{
	case WM_INITDIALOG:
		{
			double v;
			v = -2;
			SendDlgItemMessage(hDlg,IDC_BIG,DCM_SETVALUE,0,(LPARAM)&v);
			v = M_PI;
			SendDlgItemMessage(hDlg,IDC_HORIZONTAL,DCM_SETVALUE,0,(LPARAM)&v);
			v = -M_PI/2;
			SendDlgItemMessage(hDlg,IDC_VERTICAL,DCM_SETVALUE,0,(LPARAM)&v);
			v = 2;
			SendDlgItemMessage(hDlg,IDC_DISABLED,DCM_SETVALUE,0,(LPARAM)&v);
			v = 0;
			SendDlgItemMessage(hDlg,IDC_ENABLED,DCM_SETVALUE,0,(LPARAM)&v);
			SetDlgItemText(hDlg,IDC_EDIT1,L"0.00");
			
		}
		return true;
	case WM_COMMAND:
		switch (LOWORD(wPar))
		{
		case IDC_ENABLED:
			if( HIWORD(wPar) == DCN_CHANGED )
			{
				double v;
				wchar_t text[100];
				SendDlgItemMessage(hDlg,IDC_ENABLED,DCM_GETVALUE,0,(LPARAM)&v);
				v *= 180 / M_PI;
				int u,d;
				u = long(floor(v));
				d = long(fmod(floor(fabs(v)*100),100));
				wsprintf(text,L"%i.%02i",u, d);
				SetDlgItemText(hDlg,IDC_CHANGED_READ,text);
			}
			if( HIWORD(wPar) == DCN_CHANGING )
			{
				double v;
				wchar_t text[100];
				SendDlgItemMessage(hDlg,IDC_ENABLED,DCM_GETVALUE,0,(LPARAM)&v);
				v *= 180 / M_PI;
				int u,d;
				u = long(floor(v));
				d = long(fmod(floor(fabs(v)*100),100));
				wsprintf(text,L"%i.%02i",u, d);
				SetDlgItemText(hDlg,IDC_CHANGING_READ,text);
			}
			break;
		case IDC_BUTTON1:
			if( HIWORD(wPar) == BN_CLICKED )
			{
				wchar_t text[100];
				GetDlgItemText(hDlg,IDC_EDIT1,text,100);
				double v = 	_wtof(text);
				v *= M_PI / 180;
				SendDlgItemMessage(hDlg,IDC_ENABLED,DCM_SETVALUE,TRUE,(LPARAM)&v);
			}
		}
		return false;
		break;
	case WM_CLOSE:
		DestroyWindow(hDlg);
		return true;
	case WM_DESTROY:
		EndDialog(hDlg,0);
		return true;
	}
	return false;
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	RegisterDialControl(hInstance);
	return int(DialogBox(hInstance,MAKEINTRESOURCE(IDD_DIALOG1),NULL,(DLGPROC) MainProc));
}
