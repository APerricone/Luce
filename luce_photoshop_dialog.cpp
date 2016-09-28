// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#include "luce_photoshop.h"
#include "luce_preview.h"
#include "DialControl.h"
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <Commctrl.h>
#include "resource.h"
#include "colorButton.h"

// *****************************************************************************
// *** helper functions

// center the dialog around the parent or the screen
void CenterDialog(HWND hDlg);
// shortcuts to sprintf+SetDlgItemText
void SetDlgItemFloat(HWND hDlg,int idCtrl,float value,int nDecimal);
void SetDlgItemDouble(HWND hDlg,int idCtrl,double value,int nDecimal);
// shortcuts to GetDlgItemText+sscan
float GetDlgItemFloat(HWND hDlg,int idCtrl,BOOL* pbTranslated);
double GetDlgItemDouble(HWND hDlg,int idCtrl,BOOL* pbTranslated);
// shorts for SendDlgItemMessage..
void SetCheck(HWND hDlg,int idCtrl,bool check);
bool IsCheck(HWND hDlg,int idCtrl);
// code from http://msdn.microsoft.com/en-us/library/windows/desktop/hh298368(v=vs.85).aspx
HWND CreateToolTip(HWND hDlg, int toolID, PTSTR pszText);

// *****************************************************************************
// *** the class
class CPhotoshopDialog
{
public:
	//! calls DialogBox	
	bool Do(HINSTANCE hInstance,LPCSTR lpTemplateName,HWND hWndParent,LuceOptions* pOpt);
	
private:
	//! The windows callback of function
	static INT_PTR CALLBACK DlgFunc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam);

	// Message wrappers
	BOOL InitDialog();
	BOOL Paint();
	BOOL DrawItem(LPARAM lParam);
	BOOL MouseMove(WPARAM wParam,LPARAM lParam);			
	BOOL LButtonUp(WPARAM wParam,LPARAM lParam);			
	BOOL Command(WPARAM wParam,LPARAM lParam);
	BOOL HScroll(WPARAM wParam,LPARAM lParam);
	BOOL SetCursor();

	//! reads position or direction from tmpOpt
	void UpdatePos();
	//! reads sliders position from tmpOpt
	void UpdateSliders();
	//! reads all options value from dialog and puts them in tmpOpt
	void ReadParams();
	//! show/hire, enable/disable controls to reflect curent options
	void UpdateControlsStates();

	//! dialog hangle, to reduce the number of parameter for message wrappers
	HWND hDlg;
	//! information about image: is grascale
	bool bGrayScale;
	//! information about image: has alpha
	bool bAlpha;
	//! information about image: has mask
	bool bMask;
	//! source and destination for the options, 
	//! it is unchaged if user clicks cancel
	LuceOptions* pOpt;
	//! current reflection of options showed in the dialog
	LuceOptions tmpOpt;
	//! if true the command message is from a set by dialog, 
	//! not the changing made by user
	bool bUpdating;

	HCURSOR hPickCursor;
	HCURSOR hLampCursor;
	bool bPicking;

	HWND hToolTip;
	
	CLucePreview oLucePreview;
	CColorButton oColorButton;
};

// ****************************************************************************
// *** CPhotoshopLuce
bool CPhotoshopLuce::DoDialog()
{
	PlatformData *pPlatform = (PlatformData *) (pFilterRecord->platformData);	

	//MessageBox((HWND)pPlatform->hwnd,"Init preview","Debug",MB_OK);

	CPhotoshopDialog dlg;
	return dlg.Do(hInstance,MAKEINTRESOURCE(IDD_LUCEOPT),(HWND)pPlatform->hwnd,pParams);
}

// *****************************************************************************
// *** the class implementations
bool CPhotoshopDialog::Do(HINSTANCE hInstance,LPCSTR lpTemplateName,HWND hWndParent,LuceOptions* _pOpt)
{
	RegisterDialControl(hInstance);

	pOpt = _pOpt;
	bUpdating = false;
	bool v = (DialogBoxParam(hInstance,lpTemplateName,(HWND)hWndParent,DlgFunc,(LPARAM)(this)) == 1);

	UnregisterDialControl(hInstance);

	return v;
}

INT_PTR CALLBACK CPhotoshopDialog::DlgFunc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	CPhotoshopDialog *pThis = (CPhotoshopDialog*)(GetWindowLongPtr(hDlg,GWLP_USERDATA));
	if( msg == WM_INITDIALOG)
	{
		pThis = (CPhotoshopDialog*)(lParam);
		SetWindowLongPtr(hDlg,GWLP_USERDATA,(LPARAM)pThis);
	}
	if( pThis == NULL) return FALSE;
	pThis->hDlg = hDlg;
	switch (msg)
	{
		case WM_INITDIALOG:		return pThis->InitDialog();
		case WM_PAINT:			return pThis->Paint();
		case WM_DRAWITEM:		return pThis->DrawItem(lParam);
		case WM_LBUTTONDOWN:	return pThis->MouseMove(wParam,lParam);
		case WM_MOUSEMOVE:		return pThis->MouseMove(wParam,lParam);			
		case WM_LBUTTONUP:		return pThis->LButtonUp(wParam,lParam);			
		case WM_COMMAND:		return pThis->Command(wParam,lParam);
		case WM_SETCURSOR:		return pThis->SetCursor();
		case WM_HSCROLL:		return pThis->HScroll(wParam,lParam);
	}
	return FALSE;
}

BOOL CPhotoshopDialog::InitDialog()
{
	bUpdating = true;
	CenterDialog(hDlg);
	tmpOpt = *pOpt; 
	bPicking = false;
	// Get infos
	bGrayScale = CPhotoshopLuce::IsImageGrayScale();
	bAlpha = CPhotoshopLuce::HasImageAlpha();
	bMask = CPhotoshopLuce::HasImageMask();
	// enable/disable color controls
	if( bGrayScale )
	{
		ShowWindow( GetDlgItem(hDlg,IDC_VALUE), SW_SHOW );
		ShowWindow( GetDlgItem(hDlg,IDC_COLOR), SW_HIDE );
	} else
	{
		ShowWindow( GetDlgItem(hDlg,IDC_VALUE), SW_HIDE );
		ShowWindow( GetDlgItem(hDlg,IDC_COLOR), SW_SHOW );
	}
	EnableWindow( GetDlgItem(hDlg,IDC_MASKLABEL), bMask? TRUE : FALSE );
	EnableWindow( GetDlgItem(hDlg,IDC_MASK), bMask? TRUE : FALSE );
	// fill scale selector
	SendDlgItemMessage(hDlg,IDC_SCALESELECT,CB_RESETCONTENT,0,0);
	SendDlgItemMessage(hDlg,IDC_SCALESELECT,CB_ADDSTRING,0,(LPARAM)"pixel");
	SendDlgItemMessage(hDlg,IDC_SCALESELECT,CB_ADDSTRING,0,(LPARAM)"percent");
	SendDlgItemMessage(hDlg,IDC_SCALESELECT,CB_SETCURSEL,0,0);
	// setup sliders
	int sliderToSet[2] = { IDC_LIGHTINTENSITY_SLIDER, IDC_SHADOWINTENSITY_SLIDER };
	for( int i=0;i<2;i++)
	{
		SendDlgItemMessage(hDlg,sliderToSet[i],TBM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,4000));
		SendDlgItemMessage(hDlg,sliderToSet[i],TBM_SETTICFREQ,(WPARAM)1000,0);
		SendDlgItemMessage(hDlg,sliderToSet[i],TBM_SETLINESIZE,0,(LPARAM)100);
		SendDlgItemMessage(hDlg,sliderToSet[i],TBM_SETPAGESIZE,0,(LPARAM)1000);
	}
	SendDlgItemMessage(hDlg,IDC_SRC_GRAY,TBM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,255));
	SendDlgItemMessage(hDlg,IDC_SRC_GRAY,TBM_SETLINESIZE,0,(LPARAM)1);
	SendDlgItemMessage(hDlg,IDC_SRC_GRAY,TBM_SETPAGESIZE,0,(LPARAM)16);
	
	// Read options
	SetCheck(hDlg,IDC_QUADRATIC,tmpOpt.IsQuadraticAttenuation());
	SetCheck(hDlg,IDC_POINT,tmpOpt.IsPointLight());
	SetCheck(hDlg,IDC_DIRECTIONAL,tmpOpt.IsDirectionaLight());
	SetCheck(hDlg,IDC_ZERO,tmpOpt.IsZeroMode());
	SetCheck(hDlg,IDC_LIGHTFORM,tmpOpt.IsLightformMode());
	SetCheck(hDlg,IDC_LIGHTNORMALIZE,tmpOpt.IsNormalizePositive());
	SetCheck(hDlg,IDC_SHADOWNORMALIZE,tmpOpt.IsNormalizeNegative());
	SetCheck(hDlg,IDC_ADDSOURCE,tmpOpt.IsAddSource() );
	SetCheck(hDlg,IDC_ZEROTOBLACK,tmpOpt.IsSrcApplyZero() );
	if( tmpOpt.IsSrcNormalizePositive() != tmpOpt.IsSrcNormalizeNegative() )
	{
		if( tmpOpt.IsSrcNormalizePositive() )
			tmpOpt.SetSrcNormalizeNegative( );
		else
			tmpOpt.SetSrcMantainNegativeScale( );
	}
	SetCheck(hDlg,IDC_SRCNORMALIZE,tmpOpt.IsSrcNormalizePositive() );

	SetDlgItemFloat(hDlg,IDC_LIGHTINTENSITY,tmpOpt.GetPositiveIntensity()*100,2);
	SetDlgItemFloat(hDlg,IDC_SHADOWINTENSITY,tmpOpt.GetNegativeIntensity()*100,2);
		
	SetDlgItemFloat(hDlg,IDC_VALUE, tmpOpt.GetZero(0)*255,0);
	SetDlgItemFloat(hDlg,IDC_MASK, tmpOpt.GetZero(4)*255,0);
	
	UpdatePos();
	UpdateSliders();
	UpdateControlsStates();
	// Multi thread specials
	SendDlgItemMessage(hDlg,IDC_NUMTHREAD,CB_RESETCONTENT,0,0);
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	unsigned int nMax = si.dwNumberOfProcessors;
	char sThread[10];
	for(unsigned int i=0;i<nMax;i++)
	{
		_snprintf(sThread,10,"%i",i+1);
		SendDlgItemMessage(hDlg,IDC_NUMTHREAD,CB_ADDSTRING,0,(LPARAM)sThread);
	}
	SendDlgItemMessage(hDlg,IDC_NUMTHREAD,CB_SETCURSEL,pOpt->GetNumThreadsToUse()-1,0);
	hToolTip = CreateToolTip(hDlg,IDC_NUMTHREAD,
		"The suggested value is maximum value minus 1, "
		"with the maximum value the computer will be unresponsive.");
	 SendMessage(hToolTip, TTM_SETMAXTIPWIDTH, 0, 250);
	
	// Init preview
	tmpOpt.SetNumThreadsToUse(1);
	oLucePreview.Init(hDlg,ID_PREVIEW_AREA);
	oLucePreview.UpdatePreview(tmpOpt);
	oColorButton.Init(hDlg,IDC_COLOR);
	// load resource
	HINSTANCE hInstance = (HINSTANCE)(GetWindowLongPtr(hDlg,GWLP_HINSTANCE));
	hPickCursor = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_PICK));
	hLampCursor = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_LAMP));
	//
	bUpdating = false;
	return TRUE;
}

void CPhotoshopDialog::UpdatePos()
{
	int scale = (int)(SendDlgItemMessage(hDlg,IDC_SCALESELECT,CB_GETCURSEL,0,0));
	if( scale == 0)
	{
		SetDlgItemInt(hDlg,IDC_XPOS,
			long(tmpOpt.GetCenter().x * CPhotoshopLuce::GetImageSize().h),true);
		SetDlgItemInt(hDlg,IDC_YPOS,
			long(tmpOpt.GetCenter().y * CPhotoshopLuce::GetImageSize().v),true);
	} else
	{
		double ppx(tmpOpt.GetCenter().x * 100.);
		double ppy(tmpOpt.GetCenter().y * 100.);
		SetDlgItemDouble(hDlg,IDC_XPOS,ppx,2);
		SetDlgItemDouble(hDlg,IDC_YPOS,ppy,2);
	}

	double a = atan2f(tmpOpt.GetDirection().y,-tmpOpt.GetDirection().x);
	SendDlgItemMessage(hDlg,IDC_ANGLESELECT,DCM_SETVALUE,TRUE,(LPARAM)&a);

	a = a * 180.f / float(M_PI);
	SetDlgItemDouble(hDlg,IDC_ANGLE,a,0);
}

void CPhotoshopDialog::UpdateSliders()
{
	int sliderToSet[2] = { IDC_LIGHTINTENSITY_SLIDER, IDC_SHADOWINTENSITY_SLIDER };
	float valToSet[2] = { tmpOpt.GetPositiveIntensity(), tmpOpt.GetNegativeIntensity() };
	long sliderPos;
	for(int i=0;i<2;i++)
	{
		if( valToSet[i] < 0.01f ) sliderPos = 0; else
		if( valToSet[i] > 100.f ) sliderPos = 4000; else
		{
			sliderPos = long(1000.*log10((double)valToSet[i]*100));
		}
		SendDlgItemMessage(hDlg,sliderToSet[i],TBM_SETPOS,TRUE,(LPARAM)sliderPos);
	}

	float g = 
		tmpOpt.GetZero(0) * 0.30f + 
		tmpOpt.GetZero(1) * 0.59f + 
		tmpOpt.GetZero(2) * 0.11f;
	if( g < 0 ) g = 0;
	if( g > 1 ) g = 1;
	SendDlgItemMessage(hDlg,IDC_SRC_GRAY,TBM_SETPOS,TRUE,(LPARAM)(int)(g*255));
	
}

void CPhotoshopDialog::ReadParams()
{
	bUpdating = true;
	if( IsCheck(hDlg,IDC_POINT) )
	{
		tmpOpt.SetPointLight();
	} else
	{
		tmpOpt.SetDirectionaLight();
	}

	if( IsCheck(hDlg,IDC_ZERO) )
		tmpOpt.SetZeroMode(); 
	else
		tmpOpt.SetLightformMode();
	
	if( IsCheck(hDlg,IDC_ADDSOURCE) )
		tmpOpt.SetAddSource();
	else
		tmpOpt.SetNotAddSource();

	if( IsCheck(hDlg,IDC_ZEROTOBLACK) )
		tmpOpt.SetSrcApplyZero();
	else
		tmpOpt.SetSrcIgnoreZero();

	if( IsCheck(hDlg,IDC_SRCNORMALIZE) )
	{
		tmpOpt.SetSrcNormalizePositive();
		tmpOpt.SetSrcNormalizeNegative();
	}
	else
	{
		tmpOpt.SetSrcMantainPositiveScale();
		tmpOpt.SetSrcMantainNegativeScale();
	}

	if( SendDlgItemMessage(hDlg,IDC_LIGHTNORMALIZE,BM_GETCHECK,0,0) == BST_CHECKED )
		tmpOpt.SetNormalizePositive(); 
	else
		tmpOpt.SetMantainPositiveScale();
	if( SendDlgItemMessage(hDlg,IDC_SHADOWNORMALIZE,BM_GETCHECK,0,0) == BST_CHECKED )
		tmpOpt.SetNormalizeNegative(); 
	else
		tmpOpt.SetMantainNegativeScale();

	int scale = (int)(SendDlgItemMessage(hDlg,IDC_SCALESELECT,CB_GETCURSEL,0,0));
	if( tmpOpt.IsPointLight() )
	{
		double x,y;
		if( scale == 0)
		{	//pixels
			x = double((int)(GetDlgItemInt(hDlg,IDC_XPOS,NULL,TRUE))) / double(CPhotoshopLuce::GetImageSize().h);
			y = double((int)(GetDlgItemInt(hDlg,IDC_YPOS,NULL,TRUE))) / double(CPhotoshopLuce::GetImageSize().v);
		} else
		{
			x = GetDlgItemDouble(hDlg,IDC_XPOS,NULL) / 100.;
			y = GetDlgItemDouble(hDlg,IDC_YPOS,NULL) / 100.;
		}
		tmpOpt.SetCenter(x,y);
	} else
	{ 
		double a = GetDlgItemFloat(hDlg,IDC_ANGLE,NULL) * float(M_PI) / 180.f;
		SendDlgItemMessage(hDlg,IDC_ANGLESELECT,DCM_SETVALUE,TRUE,(LPARAM)&a);
		tmpOpt.SetDirection( -(float)(cos(a)),(float)(sin(a)) );
	}
	if( SendDlgItemMessage(hDlg,IDC_QUADRATIC,BM_GETCHECK,0,0) == BST_CHECKED )
		tmpOpt.SetQuadraticAttenuation();
	else
		tmpOpt.SetLinearAttenuation();

	tmpOpt.SetPositiveIntensity(GetDlgItemFloat(hDlg,IDC_LIGHTINTENSITY,NULL) / 100.f);
	tmpOpt.SetNegativeIntensity(GetDlgItemFloat(hDlg,IDC_SHADOWINTENSITY,NULL) / 100.f);

	if( bGrayScale )
	{
		float v = GetDlgItemFloat(hDlg,IDC_VALUE,NULL)/255.f;
		tmpOpt.SetZero(0,v);
		tmpOpt.SetZero(1,v);
		tmpOpt.SetZero(2,v);
	}
	tmpOpt.SetZero(3,0.f);
	tmpOpt.SetZero(4,GetDlgItemFloat(hDlg,IDC_MASK,NULL)/255.f);

	UpdateSliders();
	UpdateControlsStates();
	bUpdating = false;
}

void CPhotoshopDialog::UpdateControlsStates()
{
	bool bPoint( IsCheck(hDlg,IDC_POINT) );
	//EnableWindow( GetDlgItem(hDlg,IDC_QUADRATIC), bPoint? TRUE : FALSE );
	// show for point
	ShowWindow( GetDlgItem(hDlg,IDC_XLABEL), bPoint? SW_SHOW : SW_HIDE );
	ShowWindow( GetDlgItem(hDlg,IDC_YLABEL), bPoint? SW_SHOW : SW_HIDE );
	ShowWindow( GetDlgItem(hDlg,IDC_XPOS), bPoint? SW_SHOW : SW_HIDE );
	ShowWindow( GetDlgItem(hDlg,IDC_YPOS), bPoint? SW_SHOW : SW_HIDE );
	ShowWindow( GetDlgItem(hDlg,IDC_SCALESELECT), bPoint? SW_SHOW : SW_HIDE );
	// show for directional
	ShowWindow( GetDlgItem(hDlg,IDC_ANGLESELECT), bPoint? SW_HIDE : SW_SHOW ); // temp
	ShowWindow( GetDlgItem(hDlg,IDC_ANGLELABEL), bPoint? SW_HIDE : SW_SHOW );
	ShowWindow( GetDlgItem(hDlg,IDC_ANGLE), bPoint? SW_HIDE : SW_SHOW );
	ShowWindow( GetDlgItem(hDlg,IDC_ANGLEDEGREE), bPoint? SW_HIDE : SW_SHOW );
	// enable for zero mode
	BOOL bZero = tmpOpt.IsZeroMode()? TRUE : FALSE;
	EnableWindow( GetDlgItem(hDlg,IDC_COLOR), bZero );
	EnableWindow( GetDlgItem(hDlg,IDC_VALUE), bZero );
	EnableWindow( GetDlgItem(hDlg,IDC_PICK), bZero );
	EnableWindow( GetDlgItem(hDlg,IDC_ZEROLABEL), bZero );
	EnableWindow( GetDlgItem(hDlg,IDC_COLORLABEL), bZero );
	if( bMask )
	{
		EnableWindow( GetDlgItem(hDlg,IDC_MASKLABEL), bZero );
		EnableWindow( GetDlgItem(hDlg,IDC_MASK), bZero );	
	}
	// add src opt
	EnableWindow( GetDlgItem(hDlg,IDC_ZEROTOBLACK), bZero );
	EnableWindow( GetDlgItem(hDlg,IDC_LIGHTNORMALIZE), bZero );
	EnableWindow( GetDlgItem(hDlg,IDC_SHADOWNORMALIZE), bZero );
	
	// 	Enable for zero mode and add source
	BOOL bAdd = (tmpOpt.IsZeroMode() && tmpOpt.IsAddSource())? TRUE : FALSE;
	EnableWindow( GetDlgItem(hDlg,IDC_ZEROTOBLACK), bAdd );
	// enable for  zero mode, add source and apply zero to soyurce
	BOOL bSrcNormalize = (tmpOpt.IsZeroMode() && tmpOpt.IsAddSource() && 
							tmpOpt.IsSrcApplyZero())? TRUE : FALSE;
	EnableWindow( GetDlgItem(hDlg,IDC_SRCNORMALIZE), bSrcNormalize  );
	//
}

BOOL CPhotoshopDialog::Paint()
{
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hDlg,&ps);
	oLucePreview.Draw(hDC);
	EndPaint(hDlg, &ps);
	return FALSE;
}

BOOL CPhotoshopDialog::DrawItem(LPARAM lParam)
{
	if( oColorButton.ManageDrawItem(lParam,
		RGB(unsigned char(tmpOpt.GetZero(0)*255),
			unsigned char(tmpOpt.GetZero(1)*255),
			unsigned char(tmpOpt.GetZero(2)*255) ) ) )
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CPhotoshopDialog::MouseMove(WPARAM wParam,LPARAM lParam)
{
	if( wParam & MK_LBUTTON && !bPicking)
	{
		if( oLucePreview.ChangePos(
				GET_X_LPARAM(lParam),
				GET_Y_LPARAM(lParam), &tmpOpt) )
		{
			bUpdating = true;
			UpdatePos();
			oLucePreview.UpdatePreview(tmpOpt);
			oLucePreview.CallDraw(hDlg);
			bUpdating = false;

		}
		return TRUE;
	}
	return FALSE;
}

BOOL CPhotoshopDialog::LButtonUp(WPARAM wParam,LPARAM lParam)
{
	if( bPicking)
	{
		if( oLucePreview.Pick(
				GET_X_LPARAM(lParam),
				GET_Y_LPARAM(lParam), &tmpOpt) )
		{
			bUpdating = true;
			oColorButton.CallRedraw(hDlg);
			SetDlgItemFloat(hDlg,IDC_VALUE,tmpOpt.GetZero(0)*255.f,0);
			bUpdating = false;
			// the next setitemtext call updatepreview :)
			SetDlgItemFloat(hDlg,IDC_MASK,tmpOpt.GetZero(4)*255.f,0);
		}
		SetCheck(hDlg,IDC_PICK,false);
		bPicking = false;
		POINT p;
		GetCursorPos(&p);
		SetCursorPos(p.x,p.y);
	}
	return FALSE;
}

BOOL CPhotoshopDialog::Command(WPARAM wParam,LPARAM lParam)
{
	if( bUpdating ) return FALSE;
	int item = LOWORD (wParam);
	int cmd = HIWORD (wParam);
	bool bStrandard(false);
	switch(item) 
	{

	case IDC_XPOS:
	case IDC_YPOS:
	case IDC_ANGLE:
	case IDC_LIGHTINTENSITY:
	case IDC_SHADOWINTENSITY:
	case IDC_VALUE:
	case IDC_MASK:
		if( cmd == EN_CHANGE ) bStrandard = true;
		break;

	case IDC_POINT:
	case IDC_DIRECTIONAL:
	case IDC_QUADRATIC:
	case IDC_ZERO:
	case IDC_LIGHTFORM:
	case IDC_LIGHTNORMALIZE:
	case IDC_SHADOWNORMALIZE:
	case IDC_ADDSOURCE:
	case IDC_ZEROTOBLACK:
	case IDC_SRCNORMALIZE:
		if( cmd == BN_CLICKED ) bStrandard = true;
		break;

	case IDC_SCALESELECT:
		if( cmd == CBN_SELCHANGE )
		{
			bUpdating = true;
			UpdatePos();
			bUpdating = false;
		}
		break;

	case IDC_ANGLESELECT:
		if( cmd == DCN_CHANGING )
		{
			double v;
			SendDlgItemMessage(hDlg,IDC_ANGLESELECT,DCM_GETVALUE,0,(LPARAM)&v);
		
			v = v * 180.f / float(M_PI);
			SetDlgItemDouble(hDlg,IDC_ANGLE,v,0);
		}
		break;

	case IDC_COLOR:
		if( cmd == BN_CLICKED )
		{
			ColorServicesInfo sInfos;
			memset(&sInfos,0,sizeof(ColorServicesInfo));
			sInfos.infoSize = sizeof(ColorServicesInfo);
			sInfos.selector = plugIncolorServicesChooseColor;
			for(int i=0;i<3;i++)
				sInfos.colorComponents[i] = int16(tmpOpt.GetZero(i) * 255);
			sInfos.sourceSpace = plugIncolorServicesRGBSpace;
			sInfos.resultSpace = plugIncolorServicesRGBSpace;
			OSErr err = CPhotoshopLuce::GetFilterRecord()->colorServices(&sInfos);
			if( err == noErr )
			{
				for(int i=0;i<3;i++)
					tmpOpt.SetZero(i,sInfos.colorComponents[i]/255.f);
				oLucePreview.UpdatePreview(tmpOpt);
				oLucePreview.CallDraw(hDlg);
				oColorButton.CallRedraw(hDlg);
			}
		}
		break;
	case IDC_PICK:
		bPicking = IsCheck(hDlg,IDC_PICK);
		break;
	case IDOK:
		if( cmd == BN_CLICKED )
		{
			*pOpt = tmpOpt;
			pOpt->SetNumThreadsToUse(
				(short)SendDlgItemMessage(hDlg,IDC_NUMTHREAD,CB_GETCURSEL,0,0)+1);
			EndDialog(hDlg, 1);
		}
		break;

	case IDCANCEL:
		if( cmd == BN_CLICKED )
		{
			EndDialog(hDlg, 0);
		}
		break;
	}

	if( bStrandard )
	{
		ReadParams();
		oLucePreview.UpdatePreview(tmpOpt);
		oLucePreview.CallDraw(hDlg);
	}
	return FALSE;
}

BOOL CPhotoshopDialog::SetCursor()
{
	if( bPicking )
	{
		::SetCursor(hPickCursor);
		return TRUE;
	} else
	{
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(hDlg,&p);
		if( oLucePreview.IsInside(p.x,p.y) )
		{
			::SetCursor(hLampCursor);
			return TRUE;
		}

	}
	return FALSE;
}

BOOL CPhotoshopDialog::HScroll(WPARAM wParam,LPARAM lParam)
{
	if( bUpdating ) return FALSE;
	int idCtrl = GetDlgCtrlID((HWND)lParam);
	int idxDest = -1;	
	if( idCtrl == IDC_LIGHTINTENSITY_SLIDER ||
		idCtrl == IDC_SHADOWINTENSITY_SLIDER )
	{
		bUpdating = true;
		int sliderPos = (int)(SendMessage((HWND)lParam,TBM_GETPOS,0,0));
		float val = (float)pow( 10, sliderPos/1000.-2. );
		if( idCtrl == IDC_LIGHTINTENSITY_SLIDER ) 
		{
			tmpOpt.SetPositiveIntensity(val);
			SetDlgItemFloat(hDlg,IDC_LIGHTINTENSITY,
				tmpOpt.GetPositiveIntensity()*100,2);
		}
		if( idCtrl == IDC_SHADOWINTENSITY_SLIDER )
		{
			tmpOpt.SetNegativeIntensity(val);
			SetDlgItemFloat(hDlg,IDC_SHADOWINTENSITY,
				tmpOpt.GetNegativeIntensity()*100,2);
		}
		oLucePreview.UpdatePreview(tmpOpt);
		oLucePreview.CallDraw(hDlg);
		bUpdating = false;
	}
	if( idCtrl == IDC_SRC_GRAY )
	{
		bUpdating = true;
		int sliderPos = (int)(SendMessage((HWND)lParam,TBM_GETPOS,0,0));
		float g = (float)(sliderPos) / 255.f;
		tmpOpt.SetZero(0,g);
		tmpOpt.SetZero(1,g);
		tmpOpt.SetZero(2,g);
		if( bGrayScale )
		{
			SetDlgItemInt(hDlg,IDC_VALUE,sliderPos,FALSE);
		} else
		{
			InvalidateRect(GetDlgItem(hDlg,IDC_COLOR), NULL, FALSE );
		}
		oLucePreview.UpdatePreview(tmpOpt);
		oLucePreview.CallDraw(hDlg);
		bUpdating = false;
	}
	return FALSE;
}

// *****************************************************************************
// *** helper functions implementations
void CenterDialog(HWND hDlg)
{
    HWND hParent = GetParent(hDlg);
    if  (hParent == NULL) hParent = GetDesktopWindow();

    RECT rcParent;
    GetClientRect(hParent, &rcParent);
    ClientToScreen(hParent, (LPPOINT)&rcParent.left);
    ClientToScreen(hParent, (LPPOINT)&rcParent.right);

    RECT rcDialog;
    GetWindowRect(hDlg, &rcDialog);

    int nWidth  = rcDialog.right  - rcDialog.left;
    int nHeight = rcDialog.bottom - rcDialog.top;
	int xOrigin = rcParent.left +
		max(rcParent.right - rcParent.left - nWidth, 0) / 2;
	int yOrigin = rcParent.top +
		max(rcParent.bottom - rcParent.top - nHeight, 0) / 2;
	SetWindowPos(hDlg, NULL, xOrigin, yOrigin, nWidth, nHeight, SWP_NOZORDER | SWP_NOSIZE);
}


void SetDlgItemFloat(HWND hDlg,int idCtrl,float value,int nDecimal)
{
	char tmp[100];
	char tmp2[100];
	if( nDecimal < 0) nDecimal = 0;
	_snprintf(tmp,100,"%%.%if",nDecimal);
	_snprintf(tmp2,100,tmp,value);
	SetDlgItemText(hDlg,idCtrl,tmp2);
}

float GetDlgItemFloat(HWND hDlg,int idCtrl,BOOL* pbTranslated)
{
	char tmp2[100];
	GetDlgItemText(hDlg,idCtrl,tmp2,100);
	float v;
	if( sscanf(tmp2,"%f",&v) == 1 )
	{
		if( pbTranslated )
			*pbTranslated = TRUE;
		return v;
	}
	if( pbTranslated )
		*pbTranslated = FALSE;
	return 0.f;
}


void SetDlgItemDouble(HWND hDlg,int idCtrl,double value,int nDecimal)
{
	char tmp[100];
	char tmp2[100];
	if( nDecimal < 0) nDecimal = 0;
	_snprintf(tmp,100,"%%.%if",nDecimal);
	_snprintf(tmp2,100,tmp,value);
	SetDlgItemText(hDlg,idCtrl,tmp2);
}

double GetDlgItemDouble(HWND hDlg,int idCtrl,BOOL* pbTranslated)
{
	char tmp2[100];
	GetDlgItemText(hDlg,idCtrl,tmp2,100);
	double v;
	if( sscanf(tmp2,"%lf",&v) == 1 )
	{
		if( pbTranslated )
			*pbTranslated = TRUE;
		return v;
	}
	if( pbTranslated )
		*pbTranslated = FALSE;
	return 0.f;
}

void SetCheck(HWND hDlg,int idCtrl,bool check)
{
	SendDlgItemMessage(hDlg,idCtrl,BM_SETCHECK,
		(WPARAM)(check? BST_CHECKED:BST_UNCHECKED),0);
}

bool IsCheck(HWND hDlg,int idCtrl)
{
	return ( SendDlgItemMessage(hDlg,idCtrl,BM_GETCHECK,0,0) == BST_CHECKED );
}

HWND CreateToolTip(HWND hDlg, int toolID, PTSTR pszText)
{
    if (!toolID || !hDlg || !pszText)
    {
        return FALSE;
    }
    // Get the window of the tool.
    HWND hwndTool = GetDlgItem(hDlg, toolID);
 	HINSTANCE hInstance = (HINSTANCE)(GetWindowLongPtr(hDlg,GWLP_HINSTANCE));
   
    // Create the tooltip. g_hInst is the global instance handle.
    HWND hwndTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
                              WS_POPUP |TTS_ALWAYSTIP | TTS_BALLOON,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              hDlg, NULL, 
                              hInstance, NULL);
    
   if (!hwndTool || !hwndTip)
   {
       return (HWND)NULL;
   }                              
                              
    // Associate the tooltip with the tool.
    TOOLINFO toolInfo = { 0 };
    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.hwnd = hDlg;
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    toolInfo.uId = (UINT_PTR)hwndTool;
    toolInfo.lpszText = pszText;
    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);

    return hwndTip;
}