/**
  * FileName: exception.h
  * Author: Hawkliu
  * Version :1.0
  * Date: 2007-03-05
  * Description: ҵ������쳣��Ļ��࣬��STL��׼�쳣�̳�
  *                   ������������µ���Ҫ�����ԴӸ���̳С�
  */
#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <string>
#include <exception>
#include <iostream>

using std::string;
using std::ostream;

/**
 * ҵ���쳣����
 */
class CException : public std::exception
{
public:
    CException(int errNo, const char* szErrInfo, const char* szFile=NULL, const int iLine= 0) throw();
    CException(int errNo, const string& strErrInfo, const char* szFile=NULL, const int iLine= 0) throw();
    
    virtual ~CException() throw();

    const char* file() const throw(); // ȡ�쳣�ļ���
    int line() const throw(); // ȡ�쳣�к�
    int error() const throw(); // ��ȡ������
    virtual const char* what() const throw(); // ��ȡ����������Ϣ

protected:
    int  _errno;
    string _errinfo;
    string _file;
    int _line;
};

/**
 * �����쳣
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
 * δ֪�쳣
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
 * ���ĵ���ʧ���쳣
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
 * �����쳣
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
 * ��Ҫ���������쳣
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
 * ��ӡ�쳣��Ϣ�Ĳ���
 */
inline ostream& operator << (ostream& os, const CException& e)
{
    os<<e.error()<<":"<<e.what();
    
    return os;
}

#define EXCEPTION(err_code, err_info) CException(err_code, err_info, __FILE__, __LINE__)


#endif

