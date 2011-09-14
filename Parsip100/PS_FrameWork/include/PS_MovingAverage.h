/*
 * PS_MovingAverage.h
 *
 *  Created on: Jul 12, 2011
 *      Author: pourya
 */

#ifndef PS_MOVINGAVERAGE_H_
#define PS_MOVINGAVERAGE_H_

#include <math.h>
#include <assert.h>
#include <vector>

#define DEFAULT_WINDOW_SIZE 20

namespace PS{
namespace MATH{

template <typename T>
class CMovingAvg
{
public:
	typedef T VALUETYPE;

	CMovingAvg(int szWindow = DEFAULT_WINDOW_SIZE);
	~CMovingAvg() {SAFE_DELETE(m_lpBuffer);}

	void addValue(T v);

	T getValue(int idx) const;
	T getAverage() const;
	T getCurrent() const;
	int getWindowSize() {return m_szWindow;}

private:
	T*  m_lpBuffer;
	T	m_current;
	int m_szWindow;
	int m_idxCurrent;
};

//Implementation
template <typename T>
CMovingAvg<T>::CMovingAvg(int szWindow)
{
	assert(szWindow > 0);
	m_idxCurrent = 0;
	m_szWindow = szWindow;
	m_lpBuffer = new T[m_szWindow];
	for(int i=0; i<m_szWindow; i++)
		m_lpBuffer[i] = (T)0;
	m_current = (T)0;
}

/*
template <typename T>
CMovingAvg<T>::~MovingAvg()
{

}
*/

template<typename T>
T CMovingAvg<T>::getValue(int idx) const
{
	if(idx >=0 && idx < m_szWindow)
		return m_lpBuffer[idx];
	else
		return static_cast<T>(-1);
}

template<typename T>
void CMovingAvg<T>::addValue(T v)
{
	m_current = v;
	m_lpBuffer[m_idxCurrent] = v;
	m_idxCurrent = (m_idxCurrent + 1) % m_szWindow;
}

template<typename T>
T CMovingAvg<T>::getAverage() const
{
	T accum = 0;
	for(size_t i=0; i<m_szWindow; i++)
		accum += m_lpBuffer[i];
	return accum / static_cast<T>(m_szWindow);
}

template<typename T>
T CMovingAvg<T>::getCurrent() const
{
	return m_current;
}


}
}

#endif /* PS_MOVINGAVERAGE_H_ */
