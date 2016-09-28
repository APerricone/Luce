// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#include "colorButton.h"
#include <windowsx.h>

void CColorButton::Init(HWND hDlg,UINT _idCtl)
{
	idCtrl = _idCtl;
	GetWindowRect(GetDlgItem(hDlg, idCtrl), &rect);	
	ScreenToClient(hDlg, (LPPOINT)&rect);
	ScreenToClient(hDlg, (LPPOINT)&(rect.right));
}

bool CColorButton::ManageDrawItem(LPARAM lParam,COLORREF color)
{
	DRAWITEMSTRUCT *pDI = (DRAWITEMSTRUCT*)(lParam);
	if( pDI->CtlID != idCtrl ) return false;

	SelectBrush(pDI->hDC,GetStockBrush(DC_BRUSH));
	SelectPen(pDI->hDC,GetStockPen(DC_PEN));
	RECT r = pDI->rcItem;
	if( pDI->itemState & ODS_DISABLED )
	{
		SetDCBrushColor(pDI->hDC,0xC0C0C0);
		SetDCPenColor(pDI->hDC,0x808080);
	} else
	{
		SetDCBrushColor(pDI->hDC,color);
		if( pDI->itemState & ODS_SELECTED )
		{
			SelectPen(pDI->hDC,GetStockPen(WHITE_PEN));
		} else
			SelectPen(pDI->hDC,GetStockPen(BLACK_PEN));
	}
	Rectangle(pDI->hDC,
		pDI->rcItem.left,pDI->rcItem.top,
		pDI->rcItem.right,pDI->rcItem.bottom);

	if ( pDI->itemState & ODS_FOCUS ) 
	{
		RECT r = pDI->rcItem;
		InflateRect(&r,3,3);
	    DrawFocusRect(pDI->hDC,&r);
	}
	return TRUE;
}

void CColorButton::CallRedraw(HWND hDlg)
{
	InvalidateRect(hDlg,&rect,FALSE);
}
