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

#include <windows.h>

//! Useful class to manage a Owner draw button with color
//! \note I can create a custom control for it, like for DialControl but this 
//!	way is simpler.
class CColorButton
{
public:
	void Init(HWND hDlg,UINT idCtl);
	bool ManageDrawItem(LPARAM lParam,COLORREF color);
	void CallRedraw(HWND hDlg);
private:
	UINT idCtrl;
	RECT rect;
};
