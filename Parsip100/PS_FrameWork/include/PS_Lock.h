#pragma once
#ifndef PS_LOCK_H
#define PS_LOCK_H

#include "PS_String.h"

namespace PS{

	class CEditLock{
	private:
		bool m_bLocked;
		DAnsiStr m_strOwner;
	public:
		CEditLock()
		{
			m_bLocked = false;
		}

		CEditLock(const DAnsiStr& owner)
		{
			m_bLocked = false;
			acquire(owner);
		}

		bool acquire()
		{
			return acquire(DAnsiStr("localhost"));
		}

		bool acquire(const DAnsiStr& owner)
		{
			if(m_bLocked) return false;
			m_strOwner = owner;
			m_bLocked = true;
			return true;
		}

		void release()
		{
			m_bLocked = false;
		}

		DAnsiStr getOwner() const {return m_strOwner;}

		bool isLocked() const {return m_bLocked;}
	};
}
#endif