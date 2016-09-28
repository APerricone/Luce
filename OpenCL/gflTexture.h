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

#include "texture.h"

class gflTexture : public Texture
{
public:
	gflTexture(void);
	~gflTexture(void);

	static void InitLib();
	static void DeinitLib();

	bool Create(const char* fileName);
};
