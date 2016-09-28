// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

__kernel void Luce(
	/* 0*/	read_only image2d_t src,	
	/* 1*/	write_only image2d_t dst,
	/* 2*/  int2 lightPos)
{
	const int idx = get_global_id(0);
	const int2 dim= (int2)(get_image_width(src),get_image_height(src));
	int2 p;
	if( idx <          dim.x          ) p = (int2)(idx,0); else
	if( idx <       dim.x+dim.y       ) p = (int2)(0,idx-dim.x); else
	if( idx <    dim.x+dim.y+dim.x    ) p = (int2)(idx-dim.x-dim.y,dim.y-1); else
	if( idx < dim.x+dim.y+dim.x+dim.y ) p = (int2)(dim.x-1,idx-dim.x-dim.y-dim.x); else
		return;
		
	const sampler_t my_sampler = 
		CLK_NORMALIZED_COORDS_FALSE |
		CLK_FILTER_NEAREST |
		CLK_ADDRESS_REPEAT;

	int2 xy=lightPos;
	int2 d = p-xy;
	uint2 n = abs(d);
	int np,increment;
	int2 deltas[2];
	if(n.x>n.y)
	{
		np = n.x;
		increment = n.y;
		deltas[0] = (int2)(1,0);
		deltas[1] = (int2)(0,1);
	} else
	{
		np = n.y;
		increment = n.x;
		deltas[0] = (int2)(0,1);
		deltas[1] = (int2)(1,0);
	}
	if( d.x < 0)
	{
		deltas[0].x = -deltas[0].x;
		deltas[1].x = -deltas[1].x;
	}
	if( d.y < 0)
	{
		deltas[0].y = -deltas[0].y;
		deltas[1].y = -deltas[1].y;
	}
	int fraction= increment / 2;
	float4 sum=(float4)(0.f,0.f,0.f,0.f);
	for(int i=1;i<=np;i++)
	{
		xy+=deltas[0];
		fraction+=increment;
		if(fraction>=np)
		{
			xy+=deltas[1];
			fraction-=np;
		}
		float4 color = read_imagef(src,my_sampler,xy);
		//sum+=color;
		//color += sum / (float)(i);
		sum += color*(2.f*i-1.f);
		color += sum / ((float)(i)*(float)(i));		
		write_imagef(dst,xy, color);
	}
	if( idx == 0)
	{
		float4 color = read_imagef(src,my_sampler,lightPos);
		write_imagef(dst,lightPos, color);
	}		
}
