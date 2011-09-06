#include "PS_PixelMap.h"

#define clamp(x,a,b) ((x)<(a) ? (a) : ((x)>(b) ? (b) : (x)))
#define min(a,b) ((a)<(b) ? (a) : (b))

namespace PS{

CPixelMap :: CPixelMap( FILE* fil) {
	map=0;
	load(fil);
}

CPixelMap :: CPixelMap(const char* frame) {
	map=0;
	load(frame);
}

CPixelMap :: CPixelMap(int _w, int _h) {
	m_w= _w, m_h= _h;
	map=new UBYTE[m_w*m_h*3];
}

CPixelMap :: CPixelMap(const CPixelMap* m) {
	m_w= m->m_w, m_h= m->m_h;
	memcpy(map,m->map,m_w*m_h*3);
}

CPixelMap :: ~CPixelMap() {
	delete [] map;
}

bool CPixelMap :: load(FILE* fil) 
{
	if (map) delete[] map;
	if (fil) 
	{
		if (!readHeader(fil)) 
		{
			return false;
		} 
		else 
		{
			map=new UBYTE[m_w*m_h*3];
			fread(map,1,m_w*m_h*3,fil);
			return true;
		}
	} 
	else 
	{
		return false;
	}
}

bool CPixelMap :: load(const char* fname) 
{  
	FILE* fp = NULL;
	errno_t t = fopen_s(&fp, fname,"rb");
	if (t != 0) 
		return false;
	return load(fp);  
}

bool CPixelMap::save(const char * fname)
{
	filebuf fb;
	fb.open(fname, ios::out);

	ostream os(&fb);
		
	os << "P6" << width() << ' ' << height() << ' ' << "255\n";
	for (int y=0; y < height(); y++)
	{
		for (int x=0; x < width(); x++) 
		{
			const UBYTE * c = getPixel(x,y);
			os << c[0] << c[1] << c[2];
		}
	}

	fb.close();
	return true;
}

void CPixelMap::reset(int _w, int _h) 
{
	delete [] map;
	m_w = _w, m_h = _h;
	map = new UBYTE[m_w*m_h*3];
}

const UBYTE* CPixelMap::getPixel(int x, int y) const 
{
	x = clamp(x,0,m_w-1); 
	y = clamp(y,0,m_h-1);
	return &map[y*m_w*3+x*3];
}

UBYTE CPixelMap::red(int x, int y) const 
{
	return getPixel(x,y)[0];
}

UBYTE CPixelMap::green(int x, int y) const 
{
	return getPixel(x,y)[1];
}

UBYTE CPixelMap::blue(int x, int y) const 
{
	return getPixel(x,y)[2];
}

UBYTE CPixelMap::component(int i, int x, int y) const 
{
	return i ? (i==1 ? green(x,y) : blue(x,y)) : red(x,y);
}

void CPixelMap::backGround(UBYTE cr, UBYTE cg, UBYTE cb) 
{
	for (int x=0; x<width(); x++) 
	{
		for (int y=0; y<height(); y++) 
		{
			putPixel(x,y,cr,cg,cb);
		}
	}
}

void CPixelMap::checkers(UBYTE cr, UBYTE cg, UBYTE cb) 
{  
	const UBYTE mask = 255;
	const int D = min(height(), width())/16;
	UBYTE r,g,b;

	for (int x=0; x<width(); x++) 
	{
		for (int y=0; y<height(); y++) 
		{
			r=cr; g=cg; b=cb;
			if ((!((x/D)%2) && !((y/D)%2)) || (((x/D)%2) && ((y/D)%2)))
				r^=mask, g^=mask, b^=mask;
			putPixel(x,y,r,g,b);
		}
	}
}

void CPixelMap::whiteSpace(FILE* f) 
{
	int c;
	do  
	{
		c = fgetc(f);
		if (c == '#') while ((c=fgetc(f))!='\n');
	}
	while((c ==' ') || (c == 7) || (c == '\r') || (c=='\n'));
	ungetc(c,f);
}

int  CPixelMap::readHeader(FILE* f) 
{
	int ok=1;
	char s[2];
	fread(s,1,2,f);

	if (s[0]=='P' && s[1]=='6') 
	{
		whiteSpace(f);
		fscanf_s(f,"%d",&m_w);
		whiteSpace(f);
		fscanf_s(f,"%d",&m_h);    
		whiteSpace(f);
		fscanf_s(f,"255");
		whiteSpace(f);
	} else ok=0;
	return ok;
}

ostream& operator << (ostream& os, const CPixelMap& map) 
{
	os << "P6" << map.width() << ' ' << map.height() << ' ' << "255\n";
	for (int y=0; y<map.height(); y++)
		for (int x=0; x<map.width(); x++) {
			const UBYTE * c = map.getPixel(x,y);
			os << c[0] << c[1] << c[2];
		}
		return os;
}

}
