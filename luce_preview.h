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
#include <PIGeneral.h>

class LuceOptions;

class CLucePreview
{
public:
	CLucePreview();
	~CLucePreview();

	void Init(HWND hDlg,int idArea);
	void UpdatePreview(const LuceOptions& pOpt);
	void CallDraw(HWND hDlg);
	void Draw(HDC hDC);
	
	bool Pick(long x,long y,LuceOptions *pOpt);
	bool ChangePos(long x,long y,LuceOptions *pOpt);
	bool IsInside(long x,long y);

private:
	RECT		totalRect;
	RECT		previewRect;
	PSPixelMap	psPixelMap;
	PSPixelMask psPixelMask;
	ReadChannelDesc *pReadChannel;
	VRect readRect;

	void AdjustRect();

	unsigned char nChannel;
	bool bAlpha;
	unsigned char* currSrc;
	unsigned char* currDst;
	BufferID  srcBufferId;
	BufferID  dstBufferId;
};