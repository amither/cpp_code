#include "parameter.h"
#include <vector>
#include <sstream>



/**
 * 读取整数值(检查范围)
 */
void CParams::readIntParam(const char* buf, const char* name, int min, int max) throw (CException)
{
    char value[MAX_PARAM_LEN + 1] = {0};

    // 提取参数
    CUrlAnalyze::getParam(const_cast<char*>(buf), name, value, MAX_PARAM_LEN);   

    // 字符串检查
    if(!isDigitString(value))
    {
        throw CException(ERR_BAD_PARAM, string("Param is not a int value:") + name + "=" + value, __FILE__, __LINE__);
    }

    // 范围检查
    int iTmp = atoi(value);
    if (iTmp < min || iTmp > max)
    {
        throw CException(ERR_BAD_PARAM, string("Param is out of range:") + name + "=" + value, __FILE__, __LINE__);
    }

    mapAvps[name] = value;
}

/**
 * 读取整数值(检查范围)
 */
void CParams::readLongParam(const char* buf, const char* name, LONG min, LONG max) throw (CException)
{
    char value[MAX_PARAM_LEN + 1] = {0};

    // 提取参数
    CUrlAnalyze::getParam(const_cast<char*>(buf), name, value, MAX_PARAM_LEN);   

    // 字符串检查
    if(!isDigitString(value))
    {
        throw CException(ERR_BAD_PARAM, string("Param is not a int value:") + name + "=" + value, __FILE__, __LINE__);
    }
    
    // 范围检查
    LONG lTmp = atoll(value);
    if (lTmp < min || lTmp > max)
    {
        throw CException(ERR_BAD_PARAM, string("Param is out of range:") + name + "=" + value, __FILE__, __LINE__);
    }

    mapAvps[name] = value;
}
/**
 * 读取带符号的整数值(检查范围)
 */
void CParams::readSignedLongParam(const char* buf, const char* name, LONG min, LONG max) throw (CException)
{
    char value[MAX_PARAM_LEN + 1] = {0};

   // 提取参数
    CUrlAnalyze::getParam(const_cast<char*>(buf), name, value, MAX_PARAM_LEN);   

   string strValue =string(value);
   LONG lVualue = atoll(value);
   string strTmpVuale = toString(lVualue);
   if(strValue == "" )
   {
   	  strValue = "0"; //空的时候，后面比较转换成LONG 会是0，所以这里也做处理
   }
    if(  lVualue <=0 && strTmpVuale != strValue)
	{
			throw CException(ERR_BAD_PARAM, string("Param  is not a LONG :")+name + "=" + string(value), __FILE__, __LINE__);
    }
	if(  lVualue >0 && strTmpVuale != strValue && ("+" + strTmpVuale) != strValue) //整数可带符号也可以不带 +
	{
			throw CException(ERR_BAD_PARAM, string("Param  is not a LONG :")+name + "=" + string(value), __FILE__, __LINE__);
	}
   // 范围检查
    if (lVualue < min || lVualue > max)
    {
        throw CException(ERR_BAD_PARAM, string("Param is out of range:") + name + "=" + value, __FILE__, __LINE__);
    }
    mapAvps[name] = value;
}


/**
 * 读取字符串参数值
 */
void CParams::readStrParam(const char* buf, const char* name, int min_len, int max_len) throw (CException)
{
    char value[MAX_PARAM_LEN + 1] = {0};

    CUrlAnalyze::getParam(const_cast<char*>(buf), name, value, MAX_PARAM_LEN);   

    // 长度检查
    int iLen = strlen(value);
    
    if (iLen < min_len || iLen > max_len)
    {
        throw CException(ERR_BAD_PARAM, string("Param's length is out of range:") + name + "=" + value, __FILE__, __LINE__);
    }

    mapAvps[name] = value;
}

/**
 * 判断参数是否存在
 */
bool CParams::isExists(const char* name)
{
    return mapAvps.find(name) != mapAvps.end();
}

/**
 * 获取字符串参数
 */
string CParams::getString(const char* name)
{
    map<string, string>::iterator it = mapAvps.find(name);

    return it == mapAvps.end() ? "" : it->second;
}

/**
 * 取常量字符串值
 */
const char* CParams::operator[](const char* name)
{
    map<string, string>::iterator it = mapAvps.find(name);

    return it == mapAvps.end() ? "" : it->second.c_str();
}

/**
 * 取常量字符串值
 */
const char* CParams::operator[](const string& name)
{
    map<string, string>::iterator it = mapAvps.find(name);

    return it == mapAvps.end() ? "" : it->second.c_str();
}
    
/**
 * 获取整数参数
 */
int CParams::getInt(const char* name)
{
    map<string, string>::iterator it = mapAvps.find(name);
    
    return it == mapAvps.end() ? 0 : atoi(it->second.c_str());
}

/**
 * 获取大整数参数
 */
LONG CParams::getLong(const char* name)
{
    map<string, string>::iterator it = mapAvps.find(name);
    
    return it == mapAvps.end() ? 0 : atoll(it->second.c_str());
}

/**
 * 设置参数
 */
void CParams::setParam(const char* name, const string& value)
{
    mapAvps[name] = value;
}

/**
 * 设置参数
 */
void CParams::setParam(const char* name, const char* value)
{
    mapAvps[name] = value;
}

/**
 * 设置参数
 */
void CParams::setParam(const char* name, int value)
{
    char szTmp[128];
    snprintf(szTmp, sizeof(szTmp), "%d", value);
    mapAvps[name] = szTmp;
}

/**
 * 设置参数
 */
void CParams::setParam(const char* name, LONG value)
{
    char szTmp[128];
    snprintf(szTmp, sizeof(szTmp), "%zd", value);
    mapAvps[name] = szTmp;
}

/**
 * 字符串分割操作. 以splitter切割字符串
 * 字符串是空时返回空vector
 */
vector<string> CParams::split(const string &src, const char* splitter)
{
    vector<string> strv;
    string::size_type pos = 0, endpos = 0;
    string::size_type len = strlen(splitter);

    while(pos != string::npos && pos < src.length())
    {
        endpos = src.find(splitter, pos, len);

        string item;

        if(endpos != string::npos)
        {
            item = src.substr(pos, endpos - pos);
            pos = endpos + len;
        }
        else
        {
            item = src.substr(pos);
            pos = string::npos;
        }

        strv.push_back(item);
    }

    return strv;
}

/**
 * 标准url编码
 */
string CParams::regUrlEncode(const string& src)
{
    string result;
    string::const_iterator iter;

    for(iter = src.begin(); iter != src.end(); ++iter)
    {
        char ch = *iter;
        if ((ch >= '0' && ch <= '9') ||
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z') ||
            strchr("-_.!~*()\'", ch) != NULL)
        {
            result += ch;
        }
        else if (ch == ' ')
        {
            result += '+';
        }
        else
        {
            result += "%" + charToHex(ch);
        }
    }

    return result;
}

/**
 * 标准url解码
 */
string CParams::regUrlDecode(const string& src)
{
    string result;
    string::const_iterator iter;
    char c;

    for(iter = src.begin(); iter != src.end(); ++iter)
    {
        switch(*iter)
        {
            case '+':
                result.append(1, ' ');
                break;
            case '%':
                // Don't assume well-formed input
                if(std::distance(iter, src.end()) >= 2
                    && std::isxdigit(*(iter + 1)) && std::isxdigit(*(iter + 2)))
                {
                    c = *++iter;
                    result.append(1, hexToChar(c, *++iter));
                }
                // Just pass the % through untouched
                else
                {
                    result.append(1, '%');
                }
                break;

            default:
                result.append(1, *iter);
                break;
        }
    }

    return result;
}

/**
 * 非标准URL编码，需要指定等于符号，连接符号，转义字符串
 */
string CParams::urlEncode(const string &src, char sep /*= '&'*/, char equal /*= '='*/, const char * szEncode /*= "&=%"*/)
{
    std::string result;
    std::string::const_iterator iter;

    for(iter = src.begin() ; iter != src.end(); ++iter)
    {
        char ch = *iter;
        char szTmp[2];
        memset(szTmp, 0, sizeof(szTmp));
        snprintf(szTmp, sizeof(szTmp), "%c", ch);
        if(ch == sep || ch == equal || ch == '%')
        {
            result += "%" + charToHex(ch);
        }
        else if(strstr((char *)szEncode, szTmp) == NULL)
        {
            result += ch;
        }
        else
        {
            result += "%" + charToHex(ch);
        }
    }

    return result;
}

/**
 *
 *非标准的URL编码，只对分割符, '=', '%'进行转义
 */
string CParams::urlDecode(const string &src)
{
    string result;
    string::const_iterator iter;
    char c;

    for(iter = src.begin(); iter != src.end(); ++iter)
    {
        switch(*iter)
        {
            case '%':
                // Don't assume well-formed input
                if(std::distance(iter, src.end()) >= 2
                    && std::isxdigit(*(iter + 1)) && std::isxdigit(*(iter + 2)))
                {
                    c = *++iter;
                    result.append(1, hexToChar(c, *++iter));
                }
                // Just pass the % through untouched
                else
                {
                    result.append(1, '%');
                }
                break;

            default:
                result.append(1, *iter);
                break;
        }
    }

    return result;
}


/**
 * 将ascii值转成16进表示
 */
string CParams::charToHex(char c)
{
    string result;
    char first, second;

    first = (c & 0xF0) / 16;
    first += first > 9 ? 'A' - 10 : '0';
    second = c & 0x0F;
    second += second > 9 ? 'A' - 10 : '0';

    result.append(1, first);
    result.append(1, second);

    return result;
}

/**
 * 将16进字符表示的ascii值转成字符
 */
char CParams::hexToChar(char first, char second)
{
    int digit;
    digit = (first >= 'A' ? ((first & 0xDF) - 'A') + 10 : (first - '0'));
    digit *= 16;
    digit += (second >= 'A' ? ((second & 0xDF) - 'A') + 10 : (second - '0'));
    return static_cast<char>(digit);
}



void CParams::parse(const char* pszParams, const char* sep/* = "&"*/)
{
    vector<string> sv = split(pszParams, sep);

    for(vector<string>::const_iterator cit = sv.begin(); cit != sv.end(); ++cit)
    {
        if(cit->empty())
            continue;

        // 采用新的方式分解key=value...项目，
        // 避免value未做转义的情况下，丢失最后的'='
        string::size_type pos = cit->find('=');
        string key = cit->substr(0, pos);
        string val;

        if(pos != string::npos)
        {
            val = cit->substr(pos + 1);
            val = regUrlDecode(val);
        }

        mapAvps.insert(std::make_pair(key, val));
    }
}

string CParams::pack(const char * szSep/* = "&"*/, const char * szEqual/* = "="*/, const char * szUrlEncodeChar /*= "&%="*/) const
{
    MapType::const_iterator cit = mapAvps.begin();
    stringstream ss;

    if (cit != mapAvps.end())
    {
        // 需要对sep进行转义
        ss << cit->first << szEqual << urlEncode(cit->second, *szSep, *szEqual, szUrlEncodeChar);
        ++cit;
    }

    for (; cit != mapAvps.end(); ++cit)
    {
        // 需要对sep进行转义
        ss << szSep << cit->first << szEqual << urlEncode(cit->second, *szSep, *szEqual, szUrlEncodeChar);
    }

    return ss.str();
}




