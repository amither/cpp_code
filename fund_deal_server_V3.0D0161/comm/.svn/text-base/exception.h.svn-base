/**
  * FileName: exception.h
  * Author: Hawkliu
  * Version :1.0
  * Date: 2007-03-05
  * Description: 业务操作异常类的基类，从STL标准异常继承
  *                   而来，如果有新的需要，可以从该类继承。
  */
#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <string>
#include <exception>
#include <iostream>

using std::string;
using std::ostream;

/**
 * 业务异常基类
 */
class CException : public std::exception
{
public:
    CException(int errNo, const char* szErrInfo, const char* szFile=NULL, const int iLine= 0) throw();
    CException(int errNo, const string& strErrInfo, const char* szFile=NULL, const int iLine= 0) throw();
    
    virtual ~CException() throw();

    const char* file() const throw(); // 取异常文件名
    int line() const throw(); // 取异常行号
    int error() const throw(); // 获取错误码
    virtual const char* what() const throw(); // 获取错误描述信息

protected:
    int  _errno;
    string _errinfo;
    string _file;
    int _line;
};

/**
 * 网络异常
 */
class CNetException : public CException
{
public:
    CNetException(int errNo, const char* szErrInfo, const char* szFile=NULL, const int iLine= 0) throw() :
        CException(errNo, szErrInfo, szFile, iLine) {}
        
    CNetException(int errNo, const string& strErrInfo, const char* szFile=NULL, const int iLine= 0) throw() :
        CException(errNo, strErrInfo, szFile, iLine) {}

    virtual ~CNetException() throw() {}
};

/**
 * 未知异常
 */
class CUnknowException : public CException
{
public:
    CUnknowException(int errNo, const char* szErrInfo, const char* szFile=NULL, const int iLine= 0) throw() :
        CException(errNo, szErrInfo, szFile, iLine) {}
        
    CUnknowException(int errNo, const string& strErrInfo, const char* szFile=NULL, const int iLine= 0) throw() :
        CException(errNo, strErrInfo, szFile, iLine) {}

    virtual ~CUnknowException() throw() {}
};

/**
 * 核心调用失败异常
 */
class CCoreException : public CException
{
public:
    CCoreException(int errNo, const char* szErrInfo, const char* szFile=NULL, const int iLine= 0) throw() :
        CException(errNo, szErrInfo, szFile, iLine) {}
        
    CCoreException(int errNo, const string& strErrInfo, const char* szFile=NULL, const int iLine= 0) throw() :
        CException(errNo, strErrInfo, szFile, iLine) {}

    virtual ~CCoreException() throw() {}
};

/**
 * 解锁异常
 */
class CUnlockException : public CException
{
public:
    CUnlockException(int errNo, const char* szErrInfo, const char* szFile=NULL, const int iLine= 0) throw() :
        CException(errNo, szErrInfo, szFile, iLine) {}
        
    CUnlockException(int errNo, const string& strErrInfo, const char* szFile=NULL, const int iLine= 0) throw() :
        CException(errNo, strErrInfo, szFile, iLine) {}

    virtual ~CUnlockException() throw() {}
};

/**
 * 需要发差错处理的异常
 */
class CErrorRpcException : public CException
{
public:
    CErrorRpcException(int errNo, const char* szErrInfo, const char* szFile=NULL, const int iLine= 0) throw() :
        CException(errNo, szErrInfo, szFile, iLine) {}
        
    CErrorRpcException(int errNo, const string& strErrInfo, const char* szFile=NULL, const int iLine= 0) throw() :
        CException(errNo, strErrInfo, szFile, iLine) {}

    virtual ~CErrorRpcException() throw() {}
};



/**
 * 打印异常信息的操作
 */
inline ostream& operator << (ostream& os, const CException& e)
{
    os<<e.error()<<":"<<e.what();
    
    return os;
}

#define EXCEPTION(err_code, err_info) CException(err_code, err_info, __FILE__, __LINE__)


#endif

