//#include "stdafx.h"
#include "PS_String.h"

namespace PS
{
	//Implementations Start by CAString and then CWString
	//==================================================================================================
	CAString CAString::substr(size_t offset, size_t count) const
	{
		CTString<char> str = substrT(offset, count);
		CAString strOutput(str.ptr());
		return strOutput;
	}

	CAString& CAString::toUpper()
	{
		for (size_t i=0; i < m_length; i++)
			m_sequence[i] = toupper(m_sequence[i]);	
		return (*this);
	}

	CAString& CAString::toLower()
	{
		for (size_t i=0; i < m_length; i++)
			m_sequence[i] = tolower(m_sequence[i]);	
		return (*this);
	}

	CAString& CAString::removeStartEndSpaces()
	{	
		//Remove trailing spaces
		while(this->lastChar() == ' ')
			this->resize(m_length - 1);
		
		size_t pos = 0;
		CAString temp;
		for (size_t i=0; i < m_length; i++)
		{			
			if(m_sequence[i] != ' ')
			{
				pos = i;			 
				break;
			}
		}
		
		for (size_t i=pos; i < m_length; i++)
			temp += m_sequence[i];

		this->copyFrom(temp);
		return (*this);
	}

	CAString& CAString::trim()
	{
		CAString temp;		
		temp.reserve(m_allocated);

		//Remove trailing spaces
		while(this->lastChar() == ' ')
			this->resize(m_length - 1);

		char ch;
		for (size_t i=0; i < m_length; i++)
		{
			 ch = m_sequence[i];
			 if((ch != '\0')&&(ch != '\n')&&(ch != '\r')&&(ch != '\t'))			 
				temp += m_sequence[i];			 
		}

		this->copyFrom(temp);
		return (*this);
	}


	bool operator==(const CAString& a, const CAString& b)	
	{
		return a.isEqual(b);
	}

	bool operator!=(const CAString& a, const CAString& b)
	{
		return !a.isEqual(b);
	}
	
	std::ostream& operator <<(std::ostream& outs, const CAString& src)
	{		
		for (size_t i = 0; i < src.m_length; i++)
		{
			outs << (src.m_sequence[i]);
		}
		return outs;
	}

	std::istream& operator >>(std::istream& ins, CAString& src)
	{
		CAString strRead;
		char ch;
		strRead.reserve(512);
		while(!ins.eof())
		{
			ins >> ch;
			if((ch == '\n') || (ch == '\0'))			
				break;			
			strRead.appendFromT(ch);
		}
		src = strRead;
		return ins;
	}

	void CAString::operator=(const wchar_t src[])
	{
		this->copyFromW(src);
	}

	void CAString::operator=(const char src[])
	{
		this->copyFromT(src);
	}

	void CAString::operator +=(const CAString& src)
	{
		this->appendFrom(src);
	}

	void CAString::operator +=(const wchar_t src[])
	{
		this->appendFromW(src);
	}

	void CAString::operator +=(wchar_t src[])
	{		
		this->appendFromW(src);
	}

	void CAString::operator +=(const char src[])
	{
		this->appendFromT(src);
	}

	void CAString::operator +=(char src[])
	{
		this->appendFromT(src);
	}

	void CAString::operator +=(const char ch)
	{
		this->appendFromT(ch);
	}

	void CAString::operator +=(const wchar_t ch)
	{
		this->appendFromW(ch);
	}

	CAString CAString::operator +(const CAString& src) const
	{
		CAString x;
		x.copyFrom(*this);
		x.appendFrom(src);
		return x;
	}

	CAString CAString::operator +(const wchar_t src[]) const
	{
		CAString x;
		x.copyFrom(*this);
		x.appendFromW(src);
		return x;
	}

	CAString CAString::operator +(wchar_t src[]) const
	{
		CAString x;
		x.copyFrom(*this);
		x.appendFromW(src);
		return x;	
	}

	CAString CAString::operator +(const wchar_t ch) const
	{
		CAString x;
		x.copyFrom(*this);
		x.appendFromW(ch);
		return x;
	}

	CAString CAString::operator +(const char src[]) const
	{
		CAString x;
		x.copyFrom(*this);
		x.appendFromT(src);
		return x;
	}


	CAString CAString::operator +(char src[]) const
	{
		CAString x;
		x.copyFrom(*this);
		x.appendFromT(src);
		return x;
	}

	CAString CAString::operator +(const char ch) const
	{
		CAString x;
		x.copyFrom(*this);
		x.appendFromT(ch);
		return x;
	}


	void CAString::appendFromW(const wchar_t src[], size_t srcSize)
	{
		if(src != NULL && srcSize == 0)
			srcSize = wcslen(src);

		if((src == NULL) || (srcSize == 0)) return;

		if(capacity() < (srcSize + DEFAULT_STRING_SIZE))
			reserve(m_allocated + srcSize + DEFAULT_STRING_SIZE);

		char* pDst = &m_sequence[m_length];
		size_t ctConverted = 0;
		errno_t err = wcstombs_s(&ctConverted, pDst, m_allocated - m_length, src, srcSize);		
		m_length += srcSize;
		m_sequence[m_length] = nullChar();
	}


	void CAString::appendFromW(const wchar_t wch)
	{
		if(capacity() < DEFAULT_STRING_SIZE)
			reserve(m_allocated + DEFAULT_STRING_SIZE);

		int i;
		char *pmb = new char[MB_CUR_MAX];
		wctomb_s(&i, pmb, MB_CUR_MAX, wch);
		if(i == 1)
		{
			m_sequence[m_length] = pmb[0];				
		}
		SAFE_DELETE_ARRAY(pmb);

		m_length++;
		m_sequence[m_length] = nullChar();
	}


	size_t CAString::getInputLength(const char src[])
	{
		if(src == NULL)
			return 0;
		return strlen(src);
	}

	CAString::CAString(const CAString& src)
	{
		init();
		copyFrom(src);
	}

	CAString::CAString(const wchar_t src[], size_t srcSize /* = 0 */)
	{		
		init();
		copyFromW(src, srcSize);
	}

	CAString::CAString(const char src[], size_t srcSize)
	{	
		init();
		copyFromT(src, srcSize);
	}


	void CAString::copyFromW(const wchar_t src[], size_t srcSize)
	{
		if(src != NULL && srcSize == 0)
			srcSize = wcslen(src);

		if((src == NULL) || (srcSize == 0))
		{
			reset();
			init();
			return;
		}

		this->reserve(srcSize * 2);
		this->zero_sequence();

		size_t ctConverted = 0;
		errno_t err = wcstombs_s(&ctConverted, m_sequence, m_allocated, src, m_allocated);		
		if(err == 0)		
			m_length = srcSize;					
		else
			m_length = 0;
		m_sequence[m_length] = nullChar();
	}

	//=========================================================================================================
	//Now WideString implementations
	//=========================================================================================================
	CWString CWString::substr(size_t offset, size_t count) const
	{
		CTString<wchar_t> str = substrT(offset, count);
		CWString strOutput(str.ptr());
		return strOutput;
	}

	CWString& CWString::toUpper()
	{
		for (size_t i=0; i < m_length; i++)
			m_sequence[i] = towupper(m_sequence[i]);	
		return (*this);
	}

	CWString& CWString::toLower()
	{
		for (size_t i=0; i < m_length; i++)
			m_sequence[i] = towlower(m_sequence[i]);	
		return (*this);
	}

	bool operator==(const CWString& a, const CWString& b)
	{
		return a.isEqual(b);
	}

	bool operator!=(const CWString& a, const CWString& b)
	{
		return !a.isEqual(b);
	}

	std::ostream& operator <<(std::ostream& outs, const CWString& src)	
	{		
		CAString strWrite;
		strWrite.copyFromW(src.ptr(), src.length());
		outs << strWrite;
		/*
		for (size_t i = 0; i < strWrite.length(); i++)
		{
			outs << (strWrite[i]);
		}		
		outs << strWrite.nullChar();
		return outs;
		*/
		return outs;
	}

	std::istream& operator >>(std::istream& ins, CWString& src)
	{
		CAString strRead;
		char ch;
		strRead.reserve(512);
		while(!ins.eof())
		{
			ins >> ch;
			if((ch == '\n') || (ch == '\0'))			
				break;			
			strRead.appendFromT(ch);
		}
		if(strRead.length() > 0)
			src.copyFromA(strRead.ptr(), strRead.length());
		return ins;
	}


	void CWString::appendFromA(const char src[], size_t srcSize)
	{
		if(src != NULL && srcSize == 0)
			srcSize = strlen(src);

		if((src == NULL) || (srcSize == 0)) return;

		if(capacity() < (srcSize + DEFAULT_STRING_SIZE))
			reserve(m_allocated + srcSize + DEFAULT_STRING_SIZE);

		void* pDst = static_cast<void*>(&m_sequence[m_length]);
		size_t ctConverted;
		errno_t err = mbstowcs_s(&ctConverted, (wchar_t*)pDst, m_allocated - m_length, src, srcSize);

		m_length += srcSize;
		m_sequence[m_length] = nullChar();

	}

	void CWString::appendFromA(const char ch)
	{
		if(capacity() < DEFAULT_STRING_SIZE)
			reserve(m_allocated + DEFAULT_STRING_SIZE);

		int i;
		char *pmb = new char[MB_CUR_MAX];

		wchar_t* pwc = new wchar_t[1];
		i = mbtowc(pwc, &ch, MB_CUR_MAX);
		if(i == 1)
		{
			m_sequence[m_length] = *pwc;				
		}
		SAFE_DELETE_ARRAY(pwc);


		m_length++;
		m_sequence[m_length] = nullChar();

	}

	CWString CWString::operator +(const CWString& src) const
	{
		CWString x;
		x.copyFrom(*this);
		x.appendFrom(src);
		return x;
	}

	CWString CWString::operator +(const wchar_t src[]) const
	{
		CWString x;
		x.copyFrom(*this);
		x.appendFromT(src);
		return x;
	}

	CWString CWString::operator +(wchar_t src[]) const
	{
		CWString x;
		x.copyFrom(*this);
		x.appendFromT(src);
		return x;
	}

	CWString CWString::operator +(const wchar_t ch) const
	{
		CWString x;
		x.copyFrom(*this);
		x.appendFromT(ch);
		return x;
	}


	CWString CWString::operator +(const char src[]) const
	{
		CWString x;
		x.copyFrom(*this);
		x.appendFromA(src);
		return x;
	}

	CWString CWString::operator +(char src[]) const
	{
		CWString x;
		x.copyFrom(*this);
		x.appendFromA(src);
		return x;
	}

	CWString CWString::operator +(const char ch) const
	{
		CWString x;
		x.copyFrom(*this);
		x.appendFromA(ch);
		return x;
	}


	void CWString::operator +=(const CWString& src)
	{
		this->appendFrom(src);
	}

	void CWString::operator +=(const wchar_t src[])
	{
		this->appendFromT(src);
	}

	void CWString::operator +=(wchar_t src[])
	{
		this->appendFromT(src);
	}

	void CWString::operator +=(const char src[])
	{
		this->appendFromA(src);
	}

	void CWString::operator +=(char src[])
	{
		this->appendFromA(src);
	}

	void CWString::operator +=(const char ch)
	{
		this->appendFromA(ch);
	}

	void CWString::operator +=(const wchar_t ch)
	{
		this->appendFromT(ch);
	}


	void CWString::operator=(const char src[])
	{
		this->copyFromA(src);
	}

	void CWString::operator=(const wchar_t src[])
	{
		this->copyFromT(src);
	}

	CWString::CWString(const CWString& src)
	{
		init();
		copyFrom(src);
	}

	CWString::CWString(const char src[], size_t srcSize /* = 0 */)
	{
		init();
		copyFromA(src, srcSize);
	}

	CWString::CWString(const wchar_t src[], size_t srcSize /* = 0 */)
	{
		init();
		copyFromT(src, srcSize);
	}

	size_t CWString::getInputLength(const wchar_t src[])
	{
		if(src == NULL)
			return 0;
		return wcslen(src);
	}

	void CWString::copyFromA(const char src[], size_t srcSize)
	{
		if(src != NULL && srcSize == 0)
			srcSize = strlen(src);

		if((src == NULL) || (srcSize == 0)) 
		{
			reset();
			init();
			return;
		}

		this->reserve(srcSize * 2);
		this->zero_sequence();

		size_t ctConverted;
		errno_t err = mbstowcs_s(&ctConverted, (wchar_t*)m_sequence, m_allocated, src, srcSize);

		if(err == 0)		
			m_length = srcSize;					
		else
			m_length = 0;
		m_sequence[m_length] = nullChar();

	}

	//==================================================================
	//Global functions
	CAString toAnsiString(const CWString& strWide)
	{
		CAString strOutput;

		strOutput.copyFromW(strWide.ptr(), strWide.length());
		return strOutput;
	}

	//==================================================================
	CWString toWideString(const CAString& strAnsi)
	{
		CWString strOutput;

		strOutput.copyFromA(strAnsi.ptr(), strAnsi.length());
		return strOutput;
	}

	//==================================================================
	CAString printToAStr( const char *pFmt, ... )
	{
		va_list	vl;
		va_start( vl, pFmt );

		char	buff[1024];
		vsnprintf_s( buff, _countof(buff)-1, _TRUNCATE, pFmt, vl );

		va_end( vl );

		DAnsiStr strOut = buff;
		return strOut;
	}
	//==================================================================
	CWString printToWStr( const char *pFmt, ... )
	{
		va_list	vl;
		va_start( vl, pFmt );

		char	buff[1024];
		vsnprintf_s( buff, _countof(buff)-1, _TRUNCATE, pFmt, vl );

		va_end( vl );

		DWideStr strOut;
		strOut.copyFromA(buff);
		return strOut;
	}


}