#ifndef _COMMON_H
#define _COMMON_H

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <regex.h>
#include <pthread.h>
#include <netdb.h>
#include <errno.h>
#include "socket.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "exception.h" 

using namespace std ;
typedef vector<string> TStrVector;
typedef map<string, string> TStr2StrMap;
typedef map<int,map<std::string,std::string> > CIntStr2Map;

namespace Tools
{
	/**
	*字符转换成16进制
	*@param c 需要转换的字符
	*return string 转换完成的字符串
	*/
	//string IntoStr(const int i);
	//大写转为小写
	//string toSmallLetter(string strParaValue);
	string CharToHex(const char& c) ;

	/**
	*字符转换成16进制
	*@param str 需要转换的字符串
	*return string 转换完成的字符串
	*/

	string StrToHex(const string & str);

	/**
	*字符转换成16进制
	*@param str 需要转换的字符串
	*return string 转换完成的字符串
	*/
	string StrToHex(const unsigned char* str);

	int Bcd2ToAscii(const char *bcd,int len,int align,char *ascii);

	int AsciiToBcd2(const char *ascii,int len,char *bcd,int mode);


	/**
	*对字符串编码
	*@param oldStr 需要编码的字符串
	*@param encodeType,编译类型,1.encodeURIComponent 2.encodeURI 3.escape
	*return string 编码后的字符串
	*说明:此函数把非数字，字母的一些字符编码成16进制
	*/
	string EncodeString(const string& oldStr,int encodeType=1);

	/**
	*对字符串解码
	*@param oldStr 需要解码的字符串
	*return string 解码后的字符串
	*说明:此函数主要针对把16进制编码，把%XXX　转换成对应的字符
	*此编解码非标准URL编译码，对于大于127字符集不进行编码
	*/
	string DecodeString(const string& oldStr);

	/**
	* 把字符串按标志符分开后存放到Vector容器中
	*@param strVec 		存放字符串数组的vector
	*@param Str   	 	需要拆分的字符串
	*@param sEleSep		拆分标志符
	*/
	bool StrToVector(TStrVector& strVec,const string& Str,const string& sEleSep);

	/**
	* 把字符串按标志符分开后按名值对存放到map
	*@param outMap 		存放名值对的map
	*@param Str   	 	需要拆分的字符串
	*@param sEleSep		拆分标志符
	*@param sNvSep  	名值对之间的标志符
	*@param emptyFlag  	是否要把空值的字段存入map,比如a=&b=1,
						emptyFlag=1;a将存入map,值为空．
	*说明: Str= a=1&b=2 执行后,outMap["a"]="1" outMap["b"]="2"
	* 因后台relay返回的接口有出现 Str=&a=1&b=2 增加此子符串分解
	*/
	bool StrToMap(TStr2StrMap& outMap,const string& Str,
								string sEleSep = "&" ,string sNvSep="=" ,int emptyFlag = 0);

	void MapToStr(TStr2StrMap& inMap,string& Str,string sEleSep= "&" , string sNvSep = "=" ) ;

	/**
	*检查字符串有否由数字组成
	*@param str 需检查字符串
	*return 全是数字，由返回true ,为空或含有其它字符由返回false;
	*/
	bool  IsDigit(const char *str) ;

	char HexToChar(char firstChar,char secondChar);
	/**
	* 把unsigned 转换成 字符串
	*@param num 要转换的参数
	*/	
	template<class T> 
	string ToStr(T num) 
	{
		stringstream ss;
		ss << num;
		return ss.str();
	};


	std::string UrlDecode(const std::string& src);


	/**
	 * 判断是否是QQ号码
	 */
	 bool isQQ(const char *str);

	/**
	 * 求字符相对'0'的偏移量
	 */
	inline int CHAR_HASH(char c)
	{
	    return abs(c-'0') % 10;
	}

	/*
	获取数据库分机、分库、分表字段
	兼容小钱包消息分库算法
	*/
	string GetDbNum(const string& uin);

	bool CachestrToMap( CIntStr2Map& cachemap,const string & Str,
		string sMsgSep = "|",string sEleSep = "&" ,string sNvSep = "=",int emptyFlag = 0);

	string IntoStr(const int i);

	//大写转为小写
	string toSmallLetter(string strParaValue);

	/***************************************************************************************************
	函数名称:regex_match
	功能描述:做正则匹配运算
	输入参数:@strSrc 需要匹配的原串；strRegular 正则表达式；strErrmsg 返回的错误信息；
	cflags 匹配选项，默认用posix 扩展正则表达式
	输出参数:0: 匹配成功1:  匹配失败2:  发生错误
	返回结果:匹配结果
	作者:
	时间:2010-10-19
	其他:
	****************************************************************************************************/

	int regex_match(const string& strSrc,const string& strRegular,
		string& strErrmsg,int cflags = REG_EXTENDED);

	/***************************************************************************************************
	函数名称:ChkParaRegx
	功能描述:按照配置的正则表达式检查参数
	输入参数:toCheck需要检查的参数map格式为<参数名，参数内容>；m_mapParamRequiredRegx必选参数正则map，
			格式为<参数名,正则表达式>；m_mapParamOptionalRegx可选参数正则map，格式为<参数名,正则表达式>；
	输出参数:0: 匹配成功1:  匹配失败2:  发生错误
	返回结果:匹配结果
	作者:
	时间:2010-10-19
	其他:
	****************************************************************************************************/

	bool ChkParaRegx(TStr2StrMap &toCheck,  
	    TStr2StrMap & mapParamRequiredRegx, 
	    TStr2StrMap & mapParamOptionalRegx, 
	    string &errMsg);

	string MsgNoCount(pthread_mutex_t &m_msgNoMutex);

	int Url2IP(const string &url,string& ip,string &errMsg);
	
	int ParseUrl(const char* pszUrl,string& host,string& uri,int &servicePort,int& protocol);

	//int BgHttpCall(const string& url,string& result,string &errMsg,int iTimeOut = 5);

	bool CheckQQEmail(const string &emailbox);
    //替换敏感词
    string replace_senstive_word(const string strSRC, const string strSenW, char cRe = '*', char cEsep = '=', char cNsep = '&' );
};

#endif


