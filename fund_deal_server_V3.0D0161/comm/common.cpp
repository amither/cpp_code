#include "common.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include "md5.h"
#include <algorithm>
#include <stdarg.h>

using std::max;

/**
 * 判断是否是数字字符串
 */
bool isDigitString(const char *str)
{
    const char* p = str;

    // 略过前导空格
    while(isspace(*p))  p++;

    // 略过数字
    while(isdigit(*p))  p++;

    // 略过末尾空格
    while(isspace(*p))  p++;

    return !(*p);
}

/**
 * 将字符串转换为整数
 */
int toInt(const char* value)
{
    return value ? atoi(value) : 0;
}

/**
 * 将字符串转换为长整数
 */
LONG toLong(const char* value)
{
    return value ? atoll(value) : 0;
}

/**
 * 将字符串转换为小写
 */
string& toLower(string& str)
{
    for(string::iterator it=str.begin(); it != str.end(); ++it)
    {
        *it = tolower(*it);
    }

    return str;
}

/**
 * 将字符串转换为小写
 */
char* toLower(char* sz)
{
    for(char* p=sz; *p; p++)
    {
        *p = tolower(*p);
    }
    return sz;
}

/**
 * 将字符串转换为大写
 */
string& toUpper(string& str)
{
    for(string::iterator it=str.begin(); it != str.end(); ++it)
    {
        *it = toupper(*it);
    }

    return str;
}

/**
 * 将字符串转换为大写
 */
char* toUpper(char* sz)
{
    for(char* p=sz; *p; p++)
    {
        *p = toupper(*p);
    }
    return sz;
}

/**
 * 获取当前主机IP
 */
string getLocalHostIp()
{
    int fd, intrface;
    long ip = -1;
    char szBuf[128] = {0};
    struct ifreq buf[16];
    struct ifconf ifc;

    if((fd=socket (AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        ifc.ifc_len = sizeof buf;
        ifc.ifc_buf = (caddr_t) buf;
        if (!ioctl(fd, SIOCGIFCONF, (char*)&ifc))
        {
            intrface = ifc.ifc_len / sizeof(struct ifreq);
            while(intrface-- > 0)
            {
                if (!(ioctl (fd, SIOCGIFADDR, (char *) &buf[intrface])))
                {
                    ip=inet_addr(inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));
                    break;
                }
            }
        }
        close (fd);
    }

    // 转换为.格式
    unsigned char* pIp = (unsigned char*)(&ip);
    snprintf(szBuf, sizeof(szBuf), "%u.%u.%u.%u", pIp[0], pIp[1], pIp[2], pIp[3]);

    return szBuf;
}

/**
 * 将时间转换为系统时间
 * @input       strTime     YYYY-MM-DD HH:MM:SS
 */
time_t toUnixTime(const string& strTime)
{
    // 取年、月、日段
    int year=0, month=0, day=0, hour=0, minute=0, second=0;
    sscanf(strTime.c_str(), "%04d-%02d-%02d %02d:%02d:%02d", &year, &month, &day, &hour, &minute, &second);

    // 若日期小于1900，返回0
    if(year < 1900)     return 0;

    // 转换为当地时间
    struct  tm tm_date;
    memset(&tm_date, 0, sizeof(tm));

    tm_date.tm_year =  year - 1900;
    tm_date.tm_mon = month - 1;
    tm_date.tm_mday = day;
    tm_date.tm_hour = hour;
    tm_date.tm_min = minute;
    tm_date.tm_sec = second;

    // 转换为系统时间
    return  mktime(&tm_date);
}

/**
 * @input YYYYMMDD
 * @output YYYY-MM-DD
 */
string toDate(const string& strDate)
{
    int year, month, day;
    sscanf(strDate.c_str(), "%04d%02d%02d", &year, &month, &day);

    char szTmp[11];
    snprintf(szTmp, sizeof(szTmp), "%04d-%02d-%02d", year, month, day);

    return szTmp;
}

/**
 * 获取系统时间: YYYY-MM-DD HH:MM:SS
 */
string getSysTime()
{
    return getSysTime(time(NULL));
}

/**
 * 获取系统时间: YYYY-MM-DD HH:MM:SS
 */
string getSysTime(time_t t)
{
    struct  tm tm_now;
    localtime_r(&t, &tm_now);

    char szTmp[256];
    snprintf(szTmp, sizeof(szTmp), "%04d-%02d-%02d %02d:%02d:%02d",
                tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
                tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);

    return szTmp;
}

/**
 * 获取系统时间: YYYYMMDD
 */
string getSysDate()
{
    time_t t = time(NULL);
    struct  tm tm_now;
    localtime_r(&t, &tm_now);

    char szTmp[256];
    snprintf(szTmp, sizeof(szTmp), "%04d%02d%02d", tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday);

    return szTmp;
}

/**
 * 取时间的年部分
 *@input:   str   YYYY-MM-DD HH:MM:SS
 */
int year(const string& str)
{
    int year, month, day;
    sscanf(str.c_str(), "%04d-%02d-%02d", &year, &month, &day);
    return year;
}

/**
 * 取时间的月部分
 *@input:   str   YYYY-MM-DD HH:MM:SS
 */
int month(const string& str)
{
    int year, month, day;
    sscanf(str.c_str(), "%04d-%02d-%02d", &year, &month, &day);
    return month;
}

/**
 * 取时间的日部分
 *@input:   str   YYYY-MM-DD HH:MM:SS
 */
int day(const string& str)
{
    int year, month, day;
    sscanf(str.c_str(), "%04d-%02d-%02d", &year, &month, &day);
    return day;
}

/**
 * 取当前日期
 *@output:   string   YYYYMMDD
 */
string nowdate(const string& str)
{
    char szTmp[9];    //当天日期
    memset(szTmp, 0, sizeof(szTmp));

    int year, month, day;
    sscanf(str.c_str(), "%04d-%02d-%02d", &year, &month, &day);

    snprintf(szTmp, sizeof(szTmp), "%04d%02d%02d", year, month, day);

    return szTmp;
}

/**
 * 取上一月日期
 *@output:   string   YYYYMM
 */
string lastmonth(const string& str)
{
    char szTmp[9];    //当天日期
    int year, month;

    memset(szTmp, 0, sizeof(szTmp));

    year = atoi(str.substr(0, 4).c_str());
    month = atoi(str.substr(4, 2).c_str());

    month-=1;
    if (month == 0)
    {
        month = 12;
        year-=1;
    }

    snprintf(szTmp, sizeof(szTmp), "%04d%02d", year, month);
    return szTmp;
}

/**
 * 取下一月日期
 *@output:   string   YYYYMM
 */
string nextdate(const string& str)
{
    char szTmp[9];    //当天日期
    memset(szTmp, 0, sizeof(szTmp));

    int year, month;

    year = atoi(str.substr(0, 4).c_str());
    month = atoi(str.substr(4, 2).c_str());
    /**
     * month为月份，取值[1,12]
     * 此处计算下一月时month先减1,再加1,然后再模12，此处month-1+1写为month
     * 模12的范围为[0,11],计算结果需要加1将月份值恢复到[1,12]
     */
    month = month%12 + 1;
    if (month == 1)
    {
        year+=1;
    }

    snprintf(szTmp, sizeof(szTmp), "%04d%02d", year, month);
    return szTmp;
}

/**
 * 获取MD5摘要
 */
char* getMd5(const char *key, int len, char *szRes)
{
    if( NULL == szRes )
        return NULL;

    struct MD5Context md5c;
    unsigned char digest[64];

    MD5Init(&md5c);
    MD5Update(&md5c, (unsigned char *) key, len);
    MD5Final(digest, &md5c);

    for (int i = 0; i < 16; i++)
    {
        snprintf(szRes + i * 2, 3, "%02x", digest[i]);
    }

    szRes[32] = '\0';
    return szRes;
}

/**
 * 将字符串中的a字符替换为b字符
 */
char* replace(char* str, char a, char b)
{
    std::replace(str, str + strlen(str), a, b);

    return str;
}

/**
 * 检查是否为数字
 */
int isNumString(const char *str)
{
    const char * p = str;

    if (p == NULL)
    {
        return 0;
    }

    while (*p != '\0')
    {
        if (! isdigit(*p))
        {
            return 0;
        }
        p++;
    }

    return 1;
}

/**
 * 指针转换
 */
char *ValiStr(char *str)
{
        if (str == NULL)
                return "";
        else
                return str;
}
/**
 * 指针转换
 */
char *ValiDateStr(char *str)
{
        if (str == NULL)
                return "";
        else if(strcmp(str,"0000-00-00 00:00:00")==0)
				return "";
		else
                return str;
}

/**
 *字符串折成整数数组
 */
void split(vector <int > & list, const char * sp)
{
    char szSource[1024];
    snprintf(szSource, sizeof(szSource), "%s", sp);
    char * p = (char *)szSource;
    int iLen = strlen(sp);

    for(int iEnd = 0;iEnd < iLen; iEnd ++)
    {
        if(szSource[iEnd] == '|')
        {
            szSource[iEnd] = '\0';


            int itmp = atoi(p);
            if(itmp != 0)
            {
                list.push_back(itmp);
            }
            p = szSource + iEnd + 1;
        }
    }

    int itmp = atoi(p);
    if(itmp != 0)
    {
       list.push_back(itmp);
    }
    return ;
}

/**
 * 字符串分割操作. 以splitter切割字符串
 * 字符串是空时返回空vector
 */
vector<string> split(const string &src, const char* splitter)
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
 * 给字符串后缀一个下标
 */
string add_suffix(const char* name, int n)
{
    char szItem[128];
    snprintf(szItem, sizeof(szItem), "%s%d", name, n);

    return szItem;
}

/**
 * 将字符串去除空格
 */
string& strTrim(string& str)
{
    while (string::npos != str.find_first_of(' '))
    {
        str.erase(str.find_first_of(' '), 1);
    }

    return str;
}

/**
 * 将字符串去除空格特殊字符，只保留数字和字母
 */
string strTrimSpecial(const string& src)
{
    string result;
    string::const_iterator iter;

    for(iter = src.begin(); iter != src.end(); ++iter)
    {
        char ch = *iter;
        if ((ch >= '0' && ch <= '9') ||
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z'))
        {
            result += ch;
        }
    }

    return result;
}

int monthInterval(const string& s_time , const string& e_time)
{
	int s_year = year(s_time) ;
	int s_month = month(s_time) ;
	int e_year = year(e_time) ;
	int e_month = month(e_time) ;

	return 12 * (e_year - s_year) + (e_month - s_month) + 1 ;
}

string str_replace(string rawStr,string from,string to)
{
    if (from == "")
    {
        return rawStr;
    }
    size_t pos = rawStr.find(from);
    string retstr;
    for (;pos != string::npos;)
    {
        retstr +=rawStr.substr(0,pos) + to;
        if (pos+from.length() < rawStr.length())
        {
            rawStr = rawStr.substr(pos+from.length(),rawStr.length()-pos-from.length());
        }
        else
        {
            rawStr="";
        }
        pos = rawStr.find(from);
    }
    retstr += rawStr;
    return retstr;
};


static inline bool isspace(char ch)
{
    return (ch == ' ' || ch == '\t');
}

void InplaceTrimLeft(std::string& strValue)
{
    size_t pos = 0;
    for (size_t i = 0; i < strValue.size(); ++i)
    {
        if (isspace((unsigned char)strValue[i]))
            ++pos;
        else
            break;
    }
    if (pos > 0)
        strValue.erase(0, pos);
}

void InplaceTrimRight(std::string& strValue)
{
    size_t n = 0;
    for (size_t i = 0; i < strValue.size(); ++i)
    {
        if (isspace((unsigned char)strValue[strValue.length() - i - 1]))
            ++n;
        else
            break;
    }
    if (n != 0)
        strValue.erase(strValue.length() - n);
}

void InplaceTrim(std::string& strValue)
{
    InplaceTrimRight(strValue);
    InplaceTrimLeft(strValue);
}


void str_split(
    const std::string& strMain,
    char chSpliter,
    std::vector<std::string>& strList,
    bool bReserveNullString)
{
    strList.clear();

    if (strMain.empty())
        return;

    size_t nPrevPos = 0;
    size_t nPos;
    std::string strTemp;
    while ((nPos = strMain.find(chSpliter, nPrevPos)) != string::npos)
    {
        strTemp.assign(strMain, nPrevPos, nPos - nPrevPos);
        InplaceTrim(strTemp);
        if (bReserveNullString || !strTemp.empty())
            strList.push_back(strTemp);
        nPrevPos = nPos + 1;
    }

    strTemp.assign(strMain, nPrevPos, strMain.length() - nPrevPos);
    InplaceTrim(strTemp);
    if (bReserveNullString || !strTemp.empty())
        strList.push_back(strTemp);
}

typedef bool (*chk_special_chr_opt)(char c);

/**
 * 将字符串去除空格特殊字符,如果is_special判断为真则去除
 */
string strTrimSpecial(const string& src, chk_special_chr_opt is_not_special)
{
    string result;
    string::const_iterator iter;

    for(iter = src.begin(); iter != src.end(); ++iter)
    {
        char ch = *iter;
        if (is_not_special(ch))
        {
            result += ch;
        }
    }

    return result;
}


lcs_result::lcs_result()
  :lstart(-1),lend(-1),
  rstart(-1),rend(-1)
{
}

lcs_result::lcs_result(const lcs_result &obj)
{
    strlcs = obj.strlcs;
    lstart = obj.lstart;
    lend = obj.lend;
    rstart = obj.rstart;
    rend = obj.rend;
}

void lcs_result::clear()
{
    strlcs.clear();
    lstart = -1;
    lend = -1;
    rstart = -1;
    rend = -1;
}

bool lcs_result::valid()
{
    if (!strlcs.empty()
        && lstart != -1
        && lend != -1
        && rstart != -1
        && rend != -1)
        return true;
    else
        return false;
}

string xmlEscape(const string &str)
{
    string ret;
    size_t len = str.length();
    for (size_t i = 0; i < len; i++) {
    	switch (str[i]) {
    	case '&':ret.append("&amp;");break;
    	case '<':ret.append("&lt;");break;
    	case '>':ret.append("&gt;");break;
       case '"':ret.append("&#x22;");break;
       case '\'':ret.append("&#x27;");break;
       case '/':ret.append("&#x2f;");break;
    	default:
    		ret.push_back(str[i]);
    	}
    }
    return ret;
}


struct lcs_info {
    lcs_info()
        :type(' '),value(0)
    {}

    lcs_info(const lcs_info &obj)
    {
        type = obj.type;
        value = obj.value;
    }
    char type;
    int value;
};

typedef vector<lcs_info> row_t;
typedef vector<row_t> lcs_matrix_t;

static int compute_lcs(const string &lstr, const string &rstr, lcs_matrix_t &lcsm);
static int get_lcs(const string &lstr, lcs_matrix_t &lcsm, int lcs_len, lcs_result &res);

lcs_result lcs(const string& lstr, const string rstr)
{
    lcs_result lcs_res;
    if (lstr.empty() || rstr.empty())
        return lcs_res;

    const int llen = lstr.length();
    const int rlen = rstr.length();

    static lcs_info null_lcs_info;
    row_t row(llen+1, null_lcs_info);
    /* 初始化一个(n+1)*(m+1)的二维数组 */
    lcs_matrix_t lcsm(rlen+1, row);

    /**
     * 按算法导论中的LCS算法计算并填充二维数组
     */
    int lcs_len = compute_lcs(lstr, rstr, lcsm);
    /* 根据二维数组中计算的结果，分析获取到LCS的结果 */
    (void)get_lcs(lstr, lcsm, lcs_len, lcs_res);

    return lcs_res;
}


/**
 *
 *          _
 *          | 0                        i=0,j=0
 *          |
 * c[i][j]= < c[i-1][j-1]+1            i>0,j>0 x[i]==y[j]
 *          |
 *          | max(c[i-1][j],c[i][j-1]) i>0,j>0 x[i]!=y[j]
 *          -
 * 按上述公式进行计算，详细算法见<Introduction to Algorithms> 15.4 Longest common subsequence
 *
 * @param lstr
 * @param rstr
 * @param lcsm
 * @return 返回最长公共子序列的长度
 */
static int compute_lcs(const string &lstr, const string &rstr, lcs_matrix_t &lcsm)
{
    const int llen = lstr.length();
    const int rlen = rstr.length();
    int lcs_len = 0;

    /**示例
     * |-
     *   j 0  1  2  3
     * i   0  0  0  0
     * 0   0 |0 |0 |0
     * 1   0 \1 -1 -1
     * 2   0 |1 \2 -2
     * 3   0 |1 |2 \3
     * type为矩阵值的来源，'\'表示值为对角(i-1,j-1)值加1
     * '|'表示值继承自上方(i-1,j),'-'表示值继承自左边(i,j-1)
     */
    for (int i = 1; i <= rlen; ++i) {
        for (int j = 1; j <= llen; ++j) {
            if (rstr[i-1] == lstr[j-1]) {
                lcsm[i][j].type = '\\';
                lcsm[i][j].value = lcsm[i-1][j-1].value + 1;
                lcs_len = max(lcs_len, lcsm[i][j].value);
            } else {
                if (lcsm[i-1][j].value >= lcsm[i][j-1].value) {
                    lcsm[i][j].type = '|';
                    lcsm[i][j].value = lcsm[i-1][j].value;
                } else {
                    lcsm[i][j].type = '-';
                    lcsm[i][j].value = lcsm[i][j-1].value;
                }
            }
        }
    }

    return lcs_len;
}

static int get_lcs(const string &lstr, lcs_matrix_t &lcsm, int lcs_len, lcs_result &res)
{
    const int llen = lstr.length();
    const int rlen = lcsm.size() - 1;

    res.clear();

    /* 如果公共子序列长度为0，则直接返回 */
    if (lcs_len <= 0)
        return 0;

    bool bend = false; /* 标记是否找到公共子序列的最后一个字符在字串lstr中的结束位置 */
    bool bstart = false; /* 标记是否找到公共子序列的第一个字符在字串lstr中的开始位置 */
    vector<char> strlcs(lcs_len, '\0'); /* 用于保存公共子序列的字符 */
    int index = lcs_len-1; /* strlcs的使用索引 */
    int i = rlen; /* 二维数组lcsm的行索引 */
    int j = llen; /* 二维数组lcsm的列索引 */
    bool bsearched = false;

    /**
     * 按行列搜索找到第一个值等于公共子序列长度的位置，
     * 因为有可能有多个长度相同的公共子序列，此处找字
     * 串最开始部分的最长公共子序列
     */
    for (i = 1; i <= rlen && !bsearched; ++i) {
        for (j = 1; j <= llen && !bsearched; ++j) {
            if (lcsm[i][j].type == '\\' && lcsm[i][j].value == lcs_len) {
                bsearched = true;
                break;
            }
        }
    }

    /* 强制作一个有效性检查，以防止数组下标越界 */
    if (i > rlen)
        i = rlen;
    if (j > llen)
        j = llen;


    while (i>=1 && j>=1 && !bstart && index >= 0) {
            switch(lcsm[i][j].type)
            {
                case '\\':
                    if (!bend && lcs_len == lcsm[i][j].value) {
                        /* 记录结束位置 */
                        res.lend = j-1;
                        res.rend = i-1;
                        bend = true;
                    }
                    if (!bstart && lcsm[i][j].value == 1) {
                        /* 记录起始位置 */
                        res.lstart = j-1;
                        res.rstart = i-1;
                        bstart = true;
                    }
                    strlcs[index--] = lstr[j-1];
                    --i;
                    --j;
                    break;
                case '|':
                    --i;
                    break;
                case '-':
                    --j;
                    break;
                default:
                    return -1;
                    //break;
            }
    }

    res.strlcs = string(strlcs.begin(), strlcs.end());

    return 0;
}


/**
 * 将时间转换为自1970年1月1日以来失去时间的秒数,发生错误是返回-1
 * @param t 格式为YYYYMMDD,YYYY为年份,范围[1900,9999],MM为月份,范围[1,12],DD为日期,范围[1,31]
 * @return 自1970年1月1日以来失去时间的秒数，如发生错误返回-1
 */
time_t mktime_yyyymmdd(int t)
{
    int year = t/10000;
    int mon = (t/100)%100;
    int day = t%100;

    /**
     * 判断参数是否有效
     */
    if (TM_INVALID_YEAR(year)
        || TM_INVALID_MONTH(mon)
        || TM_INVALID_DAY(day))
        return -1;

    struct tm tm_date;

    /**
     * 将日期转换为time_t时间
     */
    memset(&tm_date, 0, sizeof(tm_date));
    tm_date.tm_year =  year - 1900;
    tm_date.tm_mon = mon - 1;
    tm_date.tm_mday = day;
    tm_date.tm_hour = 0;
    tm_date.tm_min = 0;
    tm_date.tm_sec = 0;

    return mktime(&tm_date);
}


/**
 * 计算两个日期t1与t0所相差天数
 * @param t1 格式为YYYYMMDD,YYYY为年份,范围[1900,9999],MM为月份,范围[1,12],DD为日期,范围[1,31]
 * @param t0 同t1
 * @return t1与t0所相差天数;如果出错返回INVALID_DIFFDATE(最大的int值)
 */
int diffdate_yyyymmdd(int t1, int t0)
{
    /**
     * 将日期t1转换为time_t时间
     */
    time_t t1_time = mktime_yyyymmdd(t1);
    /**
     * 当tm_year+1900大于2038时，time_t如果为4字节，则无法表示
     * mktime将出错，返回-1.time_t表示从1970经过的秒数，最大只
     * 表示到2038年
     */
    if ((time_t)(-1) == t1_time)
        return INVALID_DIFFDATE;

    /**
     * 将日期t0转换为time_t时间
     */
    time_t t0_time = mktime_yyyymmdd(t0);
    if ((time_t)(-1) == t0_time)
        return INVALID_DIFFDATE;

    /**
     * 计算差值，并将单位由秒转换为天
     */
    int diff = t1_time - t0_time;
    diff = diff/(24 * 3600);

    return diff;
}

/**
 * 计算两个日期t1与t0所相差天数
 * @param t1 格式为YYMMDD,YY为年份最后两位,范围[00,99],MM为月份,范围[1,12],DD为日期,范围[1,31]
 * @param t0 同t1
 * @return t1与t0所相差天数;如果出错返回INVALID_DIFFDATE(最大的int值)
 */
int diffdate_20yymmdd(int t1, int t0)
{
    /**
     * t1和t0的年份只有两位，yymmdd需要加上20 00 00 00
     */
    int date1 = t1 + 20000000;
    int date2 = t0 + 20000000;

    return diffdate_yyyymmdd(date1, date2);
}


string getYYYYMMDDFromStdTime(const string &time)
{
    if (time.length() != 19)
    {
        return "";
    }
    return (time.substr(0,4)+time.substr(5,2)+time.substr(8,2));
}

string cxx_printf(const char *fmt, ...)
{
    char buf[1024*10] = {0};

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf)-1, fmt, ap);
    va_end(ap);

    return buf;
}

void CreditTo18(const string & credit15, string &credit18)
{
    int Weight[17] = {7, 9, 10, 5, 8, 4, 2, 1, 6, 3, 7, 9, 10, 5, 8, 4, 2};
    char CheckSum[11] = {'1','0','X','9','8','7','6','5','4','3','2'};
   
    credit18 = credit15.substr(0,6) + "19" + credit15.substr(6);

    int sum=0;
    for(int i=0;i<17;i++)
    {
        if(credit18[i]<'0'||credit18[i]>'9') return;
        sum += (credit18[i]-'0')*Weight[i];
    }

    credit18 += string(1,CheckSum[sum % 11]);

}

bool isCreidEqual(const string &creid1,const string &creid2)
{
    string creid1up = creid1;
    string creid2up = creid2;
    toUpper(creid1up);
    toUpper(creid2up);
    string creid1_last = creid1up;
    string creid2_last = creid2up;

    if(creid1up.length() == 15)
    {
        CreditTo18(creid1up, creid1_last);
    }
    if(creid2up.length() == 15)
    {
        CreditTo18(creid2up, creid2_last);
    }
    
    if(creid1_last == creid2_last)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int ConvertCharSet(char * pSrc,char * pDest,int& iDestLen,const char * from ,const char * to)
{
	iconv_t id = iconv_open(to,from) ;
   if ((iconv_t)-1 == id) 
   {
   		iconv_close(id) ;
        return -2;
   }	
   size_t iSrcLen = strlen(pSrc) ;
   size_t destLen = iDestLen;
   if(0 != iconv(id,&pSrc,&iSrcLen,&pDest,(size_t*)&destLen))
   {
   		iconv_close(id) ;
		return -1 ;
   }
   iconv_close(id) ;
   return 0 ;
}