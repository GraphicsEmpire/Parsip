#pragma once
#ifndef PS_ERRORMANAGER_H
#define PS_ERRORMANAGER_H

#include "DSystem/include/DContainers.h"
#include "PS_String.h"

namespace PS{

#define MarkError() CErrorManager::getInstance().PushError(__FILE__, __LINE__)
#define ReportError(message) CErrorManager::getInstance().PushError(__FILE__, __LINE__, message)
#define FlushAllErrors() CErrorManager::getInstance().FlushErrors()

class CErrorManager
{
public:
	static CErrorManager& getInstance()
	{
		if (!sm_pErrorManager)
		{
			// Check for dead reference.
			if (m_isDestroyed)
				OnDeadReference();
			else
				// First call—initialize.
				Create();
		}

		return *sm_pErrorManager;
	}

	bool HasErrors() const { return (m_errorStack.size() > 0);}
	void ClearErrors();
	size_t CountErrors() const {return m_errorStack.size();}


	void PushError(const char *file, int line);
	void PushError(const char *file, int line, const char *message);
	void PushError(const char *message);
	void PopError(void) const;
	void PopMostRecentThenCleanup() const;
	void FlushErrors() const;
	bool IsOk(void) const;
private:
	CErrorManager(void);
	CErrorManager(const CErrorManager&);
	CErrorManager& operator=(const CErrorManager&);
	virtual ~CErrorManager(void);
	// Create a new CErrorManager and store a pointer to it in m_instance.
	static void Create(void);
	// Gets called if dead reference detected
	static void OnDeadReference(void);
	DAnsiStr FormatError(const char *file, int line, const char *message) const;
	DAnsiStr FormatError(const char *message) const;
protected:
	static DVec<DAnsiStr> m_errorStack;
	static CErrorManager* sm_pErrorManager;
	static bool m_isDestroyed;
};

}
#endif // ERRORMANAGER_INCLUDED

