/**
  * FileName: parameter.h
  * Author: Hawkliu
  * Version :1.0
  * Date: 2007-09-07
  * Description: ���ļ�ʵ����һ����ȡ�����������
  */
#ifndef _PARAMETER_H_
#define _PARAMETER_H_

#include <map>
#include "exception.h"
#include "error.h"
#include "UrlAnalyze.h"
#include "common.h"

using std::map;
using std::string;
using CFT::CUrlAnalyze;


/**
 * ��ȡ�����������
 */
class CParams
{
public:

    /**
     * ��ȡ����ֵ(��鷶Χ)
     */
    void readIntParam(const char* buf, const char* name, int min, int max) throw (CException);

    /**
     * ��ȡ����ֵ(��鷶Χ)
     */
    void readLongParam(const char* buf, const char* name, LONG min, LONG max) throw (CException);
	/**
	 * ��ȡ�����ŵ�����ֵ(��鷶Χ)
	 */
	void readSignedLongParam(const char* buf, const char* name, LONG min, LONG max) throw (CException);

    /**
     * ��ȡ�ַ�������ֵ(��鷶Χ)
     */
    void readStrParam(const char* buf, const char* name, int min_len, int max_len) throw (CException);

    /**
     * �жϲ����Ƿ����
     */
    bool isExists(const char* name);

    /**
     * ȡ�����ַ���ֵ
     */
    const char* operator[](const char* name);

    /**
     * ȡ�����ַ���ֵ
     */
    const char* operator[](const string& name);
    
    /**
     * ��ȡ�ַ�������ֵ
     */
    string getString(const char* name);

    /**
     * ��ȡ��������ֵ
     */
    int getInt(const char* name);

    /**
     * ��ȡ����������ֵ
     */
    LONG getLong(const char* name);

    /**
     * ���ò���
     */
    void setParam(const char* name, const string& value);

    /**
     * ���ò���
     */
    void setParam(const char* name, const char* value);

    /**
     * ���ò���
     */
    void setParam(const char* name, int value);

    /**
     * ���ò���
     */
    void setParam(const char* name, LONG value);

	/**
      * �ַ����ָ����. ��splitter�и��ַ���
      * �ַ���Ϊ��ʱ���ؿ�vector
      */
    static vector<string> split(const string &src, const char* splitter);

    /**
     * ��׼url����
     */
    static string regUrlEncode(const std::string& src);

    /**
     * ��׼url����
     */
    static string regUrlDecode(const std::string& src);

	/**
     * �Ǳ�׼URL���룬��Ҫָ�����ڷ��ţ����ӷ��ţ�ת���ַ���
    */
    static string urlEncode(const string &src, char sep = '&', char equal = '=', const char * szEncode = "&=%");

   /**
    *
    *�Ǳ�׼��URL���룬ֻ�Էָ��, '=', '%'����ת��
    */
    static string urlDecode(const string &src);

	/**
      * ��asciiֵת��16����ʾ
     */
    static string charToHex(char c);

    /**
     * ��16���ַ���ʾ��asciiֵת���ַ�
     */
    static char hexToChar(char first, char second);

	void parse(const char* pszParams, const char* sep = "&");
    void parse(const string & params, const char* sep = "&") { parse(params.c_str(), sep); }

	/**
     * ��ָ�����ַ���������
     */
    string pack(const char * szSep = "&", const char * szEqual = "=", const char * szUrlEncodeChar = "&%=") const;
    string url_pack(const char * szSep = "&", const char * szEqual = "=", const char * szUrlEncodeChar = "&%=") const
    {
        return pack(szSep, szEqual, szUrlEncodeChar);
    }

public:
	typedef map<string, string> MapType;
    /**
     * ��������ֵ�Ե�map
     */
    map<string, string>  mapAvps;
};


#endif
