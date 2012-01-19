#pragma once
#ifndef PS_ERRORMANAGER_H
#define PS_ERRORMANAGER_H


#include "PS_String.h"
#include <vector>
#include "loki/Singleton.h"

#define MAX_ERRORS_TO_KEEP 100

using namespace std;
using namespace Loki;

namespace PS{

#define MarkError() TheErrorManager::Instance().pushError(__FILE__, __LINE__)
#define ReportError(message) TheErrorManager::Instance().pushError(__FILE__, __LINE__, message)
#define ReportErrorExt(message, arg) TheErrorManager::Instance().pushErrorExt(__FILE__, __LINE__, message, arg)
#define FlushAllErrors() TheErrorManager::Instance().flushErrors()

typedef void (*FOnPopError)(const char* message);

class CErrorManager
{
public:
    CErrorManager();
    CErrorManager(const CErrorManager&);
    virtual ~CErrorManager();

    bool hasErrors() const { return (m_stkErrors.size() > 0);}
    void clearErrors();
    size_t countErrors() const {return m_stkErrors.size();}


    void pushError(const char *file, int line);
    void pushError(const char *file, int line, const char *message);
    void pushError(const char *message);
    void pushErrorExt(const char *file, int line, const char *pFmt, ...);
    void popError();
    void popMostRecentThenCleanup();
    void displayError(const char* chrError) const;
    void flushErrors();


    void setCallBackPopError(FOnPopError cb) {m_fOnPopError = cb;}
    void setWriteToFile(bool bWrite) {m_bWriteToFile = bWrite;}
    void setDisplayRightAway(bool bDisplay) {m_bDisplayRightAway = bDisplay;}

    CErrorManager& operator=(const CErrorManager&);
private:
    void pushToStack(const DAnsiStr& strError);
    DAnsiStr formatError(const char *file, int line, const char *message) const;
    DAnsiStr formatError(const char *message) const;
protected:
    bool			  m_bWriteToFile;
    bool			  m_bDisplayRightAway;
    FOnPopError		  m_fOnPopError;
    std::vector<DAnsiStr> m_stkErrors;
};


typedef SingletonHolder<CErrorManager, CreateUsingNew, PhoenixSingleton> TheErrorManager;


}
#endif // ERRORMANAGER_INCLUDED

