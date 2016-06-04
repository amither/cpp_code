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
	*�ַ�ת����16����
	*@param c ��Ҫת�����ַ�
	*return string ת����ɵ��ַ���
	*/
	//string IntoStr(const int i);
	//��дתΪСд
	//string toSmallLetter(string strParaValue);
	string CharToHex(const char& c) ;

	/**
	*�ַ�ת����16����
	*@param str ��Ҫת�����ַ���
	*return string ת����ɵ��ַ���
	*/

	string StrToHex(const string & str);

	/**
	*�ַ�ת����16����
	*@param str ��Ҫת�����ַ���
	*return string ת����ɵ��ַ���
	*/
	string StrToHex(const unsigned char* str);

	int Bcd2ToAscii(const char *bcd,int len,int align,char *ascii);

	int AsciiToBcd2(const char *ascii,int len,char *bcd,int mode);


	/**
	*���ַ�������
	*@param oldStr ��Ҫ������ַ���
	*@param encodeType,��������,1.encodeURIComponent 2.encodeURI 3.escape
	*return string �������ַ���
	*˵��:�˺����ѷ����֣���ĸ��һЩ�ַ������16����
	*/
	string EncodeString(const string& oldStr,int encodeType=1);

	/**
	*���ַ�������
	*@param oldStr ��Ҫ������ַ���
	*return string �������ַ���
	*˵��:�˺�����Ҫ��԰�16���Ʊ��룬��%XXX��ת���ɶ�Ӧ���ַ�
	*�˱����Ǳ�׼URL�����룬���ڴ���127�ַ��������б���
	*/
	string DecodeString(const string& oldStr);

	/**
	* ���ַ�������־���ֿ����ŵ�Vector������
	*@param strVec 		����ַ��������vector
	*@param Str   	 	��Ҫ��ֵ��ַ���
	*@param sEleSep		��ֱ�־��
	*/
	bool StrToVector(TStrVector& strVec,const string& Str,const string& sEleSep);

	/**
	* ���ַ�������־���ֿ�����ֵ�Դ�ŵ�map
	*@param outMap 		�����ֵ�Ե�map
	*@param Str   	 	��Ҫ��ֵ��ַ���
	*@param sEleSep		��ֱ�־��
	*@param sNvSep  	��ֵ��֮��ı�־��
	*@param emptyFlag  	�Ƿ�Ҫ�ѿ�ֵ���ֶδ���map,����a=&b=1,
						emptyFlag=1;a������map,ֵΪ�գ�
	*˵��: Str= a=1&b=2 ִ�к�,outMap["a"]="1" outMap["b"]="2"
	* ���̨relay���صĽӿ��г��� Str=&a=1&b=2 ���Ӵ��ӷ����ֽ�
	*/
	bool StrToMap(TStr2StrMap& outMap,const string& Str,
								string sEleSep = "&" ,string sNvSep="=" ,int emptyFlag = 0);

	void MapToStr(TStr2StrMap& inMap,string& Str,string sEleSep= "&" , string sNvSep = "=" ) ;

	/**
	*����ַ����з����������
	*@param str �����ַ���
	*return ȫ�����֣��ɷ���true ,Ϊ�ջ��������ַ��ɷ���false;
	*/
	bool  IsDigit(const char *str) ;

	char HexToChar(char firstChar,char secondChar);
	/**
	* ��unsigned ת���� �ַ���
	*@param num Ҫת���Ĳ���
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
	 * �ж��Ƿ���QQ����
	 */
	 bool isQQ(const char *str);

	/**
	 * ���ַ����'0'��ƫ����
	 */
	inline int CHAR_HASH(char c)
	{
	    return abs(c-'0') % 10;
	}

	/*
	��ȡ���ݿ�ֻ����ֿ⡢�ֱ��ֶ�
	����СǮ����Ϣ�ֿ��㷨
	*/
	string GetDbNum(const string& uin);

	bool CachestrToMap( CIntStr2Map& cachemap,const string & Str,
		string sMsgSep = "|",string sEleSep = "&" ,string sNvSep = "=",int emptyFlag = 0);

	string IntoStr(const int i);

	//��дתΪСд
	string toSmallLetter(string strParaValue);

	/***************************************************************************************************
	��������:regex_match
	��������:������ƥ������
	�������:@strSrc ��Ҫƥ���ԭ����strRegular ������ʽ��strErrmsg ���صĴ�����Ϣ��
	cflags ƥ��ѡ�Ĭ����posix ��չ������ʽ
	�������:0: ƥ��ɹ�1:  ƥ��ʧ��2:  ��������
	���ؽ��:ƥ����
	����:
	ʱ��:2010-10-19
	����:
	****************************************************************************************************/

	int regex_match(const string& strSrc,const string& strRegular,
		string& strErrmsg,int cflags = REG_EXTENDED);

	/***************************************************************************************************
	��������:ChkParaRegx
	��������:�������õ�������ʽ������
	�������:toCheck��Ҫ���Ĳ���map��ʽΪ<����������������>��m_mapParamRequiredRegx��ѡ��������map��
			��ʽΪ<������,������ʽ>��m_mapParamOptionalRegx��ѡ��������map����ʽΪ<������,������ʽ>��
	�������:0: ƥ��ɹ�1:  ƥ��ʧ��2:  ��������
	���ؽ��:ƥ����
	����:
	ʱ��:2010-10-19
	����:
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
    //�滻���д�
    string replace_senstive_word(const string strSRC, const string strSenW, char cRe = '*', char cEsep = '=', char cNsep = '&' );
};

#endif


