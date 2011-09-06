//*****************************************************************************************
//Computer Graphics Lab
//Author : Pourya Shirazian
//*****************************************************************************************
#pragma once;
#ifndef CPIXELMAP_H
#define CPIXELMAP_H

#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;

namespace PS{

typedef unsigned char UBYTE;
//**************************************************************
//CPixelMap manages a bitmap, load/save from/to PPM files.
//Put and Get Pixel Values
//Add and Subtract from pixel values
//Get the pointer to the buffer of pixels
//Get and Set the dimensions of bitmap
//**************************************************************
class CPixelMap
{
public:

	CPixelMap( const char* );
	CPixelMap( FILE* fil);
	CPixelMap( int, int );
	CPixelMap( const CPixelMap* );
	~CPixelMap();

	bool load( const char* );
	bool load( FILE* fil );
	bool save(const char * fname);

	inline void putPixel ( int x, int y, UBYTE r, UBYTE g, UBYTE b )
	{
		if (x>=0 && x<m_w && y>=0 && y<m_h)
			map[y*m_w*3+x*3] = r,map[y*m_w*3+x*3+1]=g,map[y*m_w*3+x*3+2]=b;
	}

	inline void addpixel ( int x, int y, UBYTE r, UBYTE g, UBYTE b )
	{
		if (x>=0 && x<m_w && y>=0 && y<m_h)
			map[y*m_w*3+x*3] += r,map[y*m_w*3+x*3+1]+=g,map[y*m_w*3+x*3+2]+=b;
	}

	inline void addipixel ( int x, int y, int r, int g, int b ) 
	{
		int ctPixel = 0;
		if (x>=0 && x<m_w && y>=0 && y<m_h)
		{
			ctPixel = y*m_w*3+x*3;
			map[ctPixel + 0] =((map[ctPixel + 0]+r)<0) ? 0 : map[ctPixel + 0]+r;
			map[ctPixel + 1] =((map[ctPixel + 1]+g)<0) ? 0 : map[ctPixel + 1]+g;
			map[ctPixel + 2] =((map[ctPixel + 2]+b)<0) ? 0 : map[ctPixel + 2]+b;
		}
	}

	inline void subpixel ( int x, int y, int r, int g, int b ) 
	{
		if (x>=0 && x<m_w && y>=0 && y<m_h)
			map[y*m_w*3+x*3+0]=((map[y*m_w*3+x*3+0]-r)<0) ? 0 : map[y*m_w*3+x*3+0]-r;
		map[y*m_w*3+x*3+1]=((map[y*m_w*3+x*3+1]-g)<0) ? 0 : map[y*m_w*3+x*3+1]-g;
		map[y*m_w*3+x*3+2]=((map[y*m_w*3+x*3+2]-b)<0) ? 0 : map[y*m_w*3+x*3+2]-b;
	}

	const UBYTE* getPixel ( int, int ) const;
	UBYTE red ( int, int ) const;  
	UBYTE green ( int, int ) const;  
	UBYTE blue ( int, int ) const;  
	UBYTE component ( int, int,  int ) const;  

	int width() const { return m_w; }
	int height() const { return m_h; }

	const UBYTE* buffer() const{ return map; }
	void reset( int, int );
	void checkers( UBYTE = 0, UBYTE = 0, UBYTE = 0);
	void backGround(UBYTE cr, UBYTE cg, UBYTE cb);

	//  void stripes( UBYTE = 0, UBYTE = 0, UBYTE = 0);  
	//  const CPixelMap& difference(const CPixelMap&);

	friend ostream& operator << ( ostream&, const CPixelMap&);


private:
	int readHeader ( FILE* );
	void whiteSpace( FILE* );
	UBYTE * map;
	int m_w, m_h;
};

}
#endif