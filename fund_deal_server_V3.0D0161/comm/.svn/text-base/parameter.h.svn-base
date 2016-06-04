/**
  * FileName: parameter.h
  * Author: Hawkliu
  * Version :1.0
  * Date: 2007-09-07
  * Description: 本文件实现了一个读取输入参数的类
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
 * 读取输入参数的类
 */
class CParams
{
public:

    /**
     * 读取整数值(检查范围)
     */
    void readIntParam(const char* buf, const char* name, int min, int max) throw (CException);

    /**
     * 读取整数值(检查范围)
     */
    void readLongParam(const char* buf, const char* name, LONG min, LONG max) throw (CException);
	/**
	 * 读取带符号的整数值(检查范围)
	 */
	void readSignedLongParam(const char* buf, const char* name, LONG min, LONG max) throw (CException);

    /**
     * 读取字符串参数值(检查范围)
     */
    void readStrParam(const char* buf, const char* name, int min_len, int max_len) throw (CException);

    /**
     * 判断参数是否存在
     */
    bool isExists(const char* name);

    /**
     * 取常量字符串值
     */
    const char* operator[](const char* name);

    /**
     * 取常量字符串值
     */
    const char* operator[](const string& name);
    
    /**
     * 获取字符串参数值
     */
    string getString(const char* name);

    /**
     * 获取整数参数值
     */
    int getInt(const char* name);

    /**
     * 获取大整数参数值
     */
    LONG getLong(const char* name);

    /**
     * 设置参数
     */
    void setParam(const char* name, const string& value);

    /**
     * 设置参数
     */
    void setParam(const char* name, const char* value);

    /**
     * 设置参数
     */
    void setParam(const char* name, int value);

    /**
     * 设置参数
     */
    void setParam(const char* name, LONG value);

	/**
      * 字符串分割操作. 以splitter切割字符串
      * 字符串为空时返回空vector
      */
    static vector<string> split(const string &src, const char* splitter);

    /**
     * 标准url编码
     */
    static string regUrlEncode(const std::string& src);

    /**
     * 标准url解码
     */
    static string regUrlDecode(const std::string& src);

	/**
     * 非标准URL编码，需要指定等于符号，连接符号，转义字符串
    */
    static string urlEncode(const string &src, char sep = '&', char equal = '=', const char * szEncode = "&=%");

   /**
    *
    *非标准的URL编码，只对分割符, '=', '%'进行转义
    */
    static string urlDecode(const string &src);

	/**
      * 将ascii值转成16进表示
     */
    static string charToHex(char c);

    /**
     * 将16进字符表示的ascii值转成字符
     */
    static char hexToChar(char first, char second);

	void parse(const char* pszParams, const char* sep = "&");
    void parse(const string & params, const char* sep = "&") { parse(params.c_str(), sep); }

	/**
     * 按指定的字符连接起来
     */
    string pack(const char * szSep = "&", const char * szEqual = "=", const char * szUrlEncodeChar = "&%=") const;
    string url_pack(const char * szSep = "&", const char * szEqual = "=", const char * szUrlEncodeChar = "&%=") const
    {
        return pack(szSep, szEqual, szUrlEncodeChar);
    }

public:
	typedef map<string, string> MapType;
    /**
     * 参数属性值对的map
     */
    map<string, string>  mapAvps;
};


#endif
