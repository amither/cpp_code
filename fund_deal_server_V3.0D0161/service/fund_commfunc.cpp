/**
  * FileName: fund_commfunc.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-8-16
  * Description: 基金交易服务 公共函数 源文件
  */

#include "globalconfig.h"
#include "fund_common.h"
#include "fund_struct_def.h"
#include "fund_commfunc.h"
#include "md5.h"
#include "bill_no.h"
#include <string>

#include "dbsign.h"
#include "db_code.h"

// 全局配置指针
extern GlobalConfig* gPtrConfig;
extern CftLog* gPtrSysLog;

// 连接基金数据库句柄
extern CMySQL  *gPtrFundDB;

//ckv cache 访问句柄
extern CkvSvrOperator *gCkvSvrOperator;
extern CkvSvrOperator *gSubAccCkvSvrOperator;

//fundfetchserver 链接句柄
extern CRpcWrapper* gPtrFundFetchRpc; 

struct ckv_key_t {
    int key_no;
    const char* key_prefix;
};

/**
 * key_no的定义参考CKV_KEY_NO_TYPE的说明
 * key前缀为申请key时写的固定值
 */
ckv_key_t g_ckv_key_prefix[CKV_KEY_END]={
    //注释的格式为
    // 编号 CKV_KEY_NO_TYPE的定义 说明 key的格式
    {0,""},// 0
    {CKV_KEY_UIN,""},// 1 CKV_KEY_UIN 用户基本信息 uin
    {CKV_KEY_TRADE_ID,""},// 2 CKV_KEY_TRADE_ID 用户绑定的所有基金公司列表 trade_id
    {CKV_KEY_TOTAL_PROFIT,"total_profit_"},// 3 CKV_KEY_TOTAL_PROFIT 用户累计收益 前缀+trade_id
    {CKV_KEY_FUND_SUPPORT_SP_ALL,"fund_support_sp_all"},// 4 CKV_KEY_FUND_SUPPORT_SP_ALL 支持的所有基金公司 前缀
    {CKV_KEY_SPID_FUNDCODE_CONF,"spid_fundcode_conf_"},// 5 CKV_KEY_SPID_FUNDCODE_CONF 基金代码信息 前缀+spid+"_"+fund_code
    {CKV_KEY_FUND_SUPPORT_BANK,"fund_support_bank_"},// 6 CKV_KEY_FUND_SUPPORT_BANK 银行信息 前缀+bank_type
    {CKV_KEY_FUND_SUPPORT_BANK_ALL,"fund_support_bank_all"},// 7 CKV_KEY_FUND_SUPPORT_BANK_ALL 全部银行信息 前缀
    {CKV_KEY_UID,""},// 8 CKV_KEY_UID  交易记录 uid 该key被废弃
    {CKV_KEY_PROFIT_RATE,"profit_rate_"},// 9 CKV_KEY_PROFIT_RATE 基金代码最新收益率信息 前缀+spid+fund_code
    {CKV_KEY_PROFIT_RECORD,"profit_record_"},// 10 CKV_KEY_PROFIT_RECORD 益记录流水 前缀+trade_id
    {CKV_KEY_PAY_CARD,"pay_card_"},// 11 CKV_KEY_PAY_CARD 首次支付卡信息 前缀+uin
    {CKV_KEY_FUND_PREPAY,"fund_prepay_"},// 12 CKV_KEY_FUND_PREPAY 预支付单信息 前缀+Flistid
    {CKV_KEY_FUND_TRADE,"fund_trade_"},// 13 CKV_KEY_FUND_TRADE 基金交易记录,单条 前缀+Flistid
    {CKV_KEY_BI,"_BI"},// 14 CKV_KEY_BI T日份额信息,该kEY作为后缀添加 trade_id+前缀
    {CKV_KEY_FUND_TOTALACC_2,"fund_totalacc_2_"},// 15 CKV_KEY_FUND_TOTALACC_2 总账户份额 前缀+trade_id
    {CKV_KEY_MULTI_PROFIT_RECORD,"multi_profit_record_"},// 16 CKV_KEY_MULTI_PROFIT_RECORD  基金代码最新21条收益率信息 前缀+spid+fund_code
    {CKV_KEY_HIGHTEST_PROFIT_RATE_SP,"highest_profit_rate_sp"},// 17 CKV_KEY_HIGHTEST_PROFIT_RATE_SP 收益率最高的基金公司信息 前缀
    {CKV_KEY_MULTI_SP_WHITE_USER,"multi_sp_white_user_"},// 18 CKV_KEY_MULTI_SP_WHITE_USER 多基金体验用户白名单 前缀+uin
    {CKV_KEY_LCT_ACTION_LIST,"lct_action_list_"},// 19 CKV_KEY_LCT_ACTION_LIST 用户参与的活动列表信息 前缀+uin
    {CKV_KEY_LCT_ACTION,"lct_action_"},// 20 CKV_KEY_LCT_ACTION 用户参与活动的具体信息 前缀+act_id+uin
    {CKV_KEY_CLOSE_FUND_CYCLE,"close_fund_cycle_"},// 21 CKV_KEY_CLOSE_FUND_CYCLE 定期产品运作周期数据 前缀+Ffund_code+Fdate
    {CKV_KEY_USER_DYNAMIC_INFO,"user_dynamic_info_"},// 22 CKV_KEY_USER_DYNAMIC_INFO 用户动态信息 前缀+trade_id
    {CKV_KEY_MOBILE_INFO,"mobile_info_"},// 23 CKV_KEY_MOBILE_INFO 合约机信息表 前缀+uin
    {CKV_KEY_FUND_CLOSE_TRANS,"fund_close_trans_"},// 24 CKV_KEY_FUND_CLOSE_TRANS 用户定期交易购买列表 前缀+trade_id+fund_code
    {CKV_KEY_FUND_USER_ACC,"fund_user_acc_"},// 25 CKV_KEY_FUND_USER_ACC 用户购买有份额基金和期次列表 前缀+trade_id
    {CKV_KEY_USR_BILLID,""},// 26 CKV_KEY_USR_BILLID 用户周报信息
    {CKV_KEY_CHARGE_INFO_PC,"fund_charge_info_pc_"},// 27 CKV_KEY_CHARGE_INFO_PC = 27 用户PC充值单信息 前缀+qr_code_token
    {CKV_KEY_FUND_TRANS_DATE,"fund_trans_date_"},// 28 CKV_KEY_FUND_TRANS_DATE 基金交易日信息 前缀+Fdate
    {CKV_KEY_USER_LATEST_FUND_TRADE,"user_latest_fund_trade_"},// 29  CKV_KEY_USER_LATEST_FUND_TRADE 用户最新的21条交易列表 前缀+trade_id
    {CKV_KEY_MQQ_ACC_TOKEN,"mqq_msg_access_token"},// 30 CKV_KEY_MQQ_ACC_TOKEN 手Q理财通发送生活服务号消息的access_token 前缀
    {CKV_KEY_FUND_BALANCE_CONFIG,"fund_balance_config_1"},// 31 CKV_KEY_FUND_BALANCE_CONFIG 余额配置信息表 前缀
    {CKV_KEY_PC_CHARGE_WHITELIST,"pc_charge_white_list_"},// 32 CKV_KEY_PC_CHARGE_WHITELIST 用户是否拥有理财通余额PC充值的权限 前缀+uin
    {CKV_KEY_IDXPAGE_ACTIVE,"active_"},// 33 CKV_KEY_IDXPAGE_ACTIVE 用户是否有首页活动的权限 前缀+uin+"_"+active_name
    {CKV_KEY_WHITE_LIST,"white_list_"},// 34 CKV_KEY_WHITE_LIST 理财通白名单功能 前缀+"业务描述"
    {CKV_KEY_ALL_ONE_DAY_PROFIT_RECORD,"all_last_profit_record"}, // 35 CKV_KEY_ALL_ONE_DAY_PROFIT_RECORD 所有基金最近一天收益率 前缀
    {CKV_KEY_ALL_SPID_CONF,"fund_spid_conf_"},//36所有基金公司信息
    {CKV_KEY_OPENID,"openid_"},//36所有基金公司信息
    {CKV_KEY_UNCONFIRM,"unconfirm_"},//38 用户未确认金额
    {CKV_KEY_TDAY,"tday_"},//39交易日信息
    {CKV_KEY_UNFINISH_INDEX,"unfinish_index_"}, //40未完成指数
    {CKV_KEY_CASH_IN_TRANSIT,"cash_in_transit_"} //44在途数据
};


/**
 * 根据key no获取key前缀
 * @param key_no 
 * @return 
 */
string get_ckv_key_prefix(int key_no)
{
    for (int i = 0; i < CKV_KEY_END; ++i) {
        if (key_no == g_ckv_key_prefix[i].key_no)
            return g_ckv_key_prefix[i].key_prefix;
    }

    throw EXCEPTION(ERR_INVALID_CKV_KEYNO, "ckv key-no&key-prefix no config");
}

/**
 * 去除key前缀
 * @param key_no 
 * @param key 
 * @return 
 */
string adapt_ckv_key(int key_no,const string &key)
{   
    /* 检查key no是否合法 */
    if (key_no < CKV_KEY_UIN || key_no >= CKV_KEY_END)
        throw EXCEPTION(ERR_INVALID_CKV_KEYNO, "invalid ckv key no");   

    /* 如果key前缀为空，则直接返回key */
    string key_prefix = get_ckv_key_prefix(key_no);
    if (key_prefix.empty())
        return key;
    
    if (CKV_KEY_BI == key_no) {
        /**
         * CKV_KEY_BI是后缀，做特殊处理
         */        
        string::size_type pos = key.rfind(key_prefix);
        if (string::npos == pos)
            return key;
        else
            return key.substr(0, pos);
    } else {
        /**
         * 如果key中未找到前缀则说明错误
         */        
        string::size_type pos = key.find(key_prefix);
        if (0 == pos) {
            return key.substr(key_prefix.length());
        } else {
            //throw EXCEPTION(ERR_INVALID_CKV_KEY, "invalid ckv key");
            //如果找不到正确前缀则直接返回key
            return key;
        }
    }
}

string StrUpper(const string &str)
{
    string dst;

    dst = str;
    for (size_t i = 0; i < dst.size(); i++)
    {
        if (dst[i] >= 'a' && dst[i] <= 'z')
        {
            dst[i] -= 'a' - 'A';
        }
    }

    return dst;
}

int Sdb1(int s)
{
        char buf[8], sz[16];
        bzero(buf, 8);
        bzero(sz, 16);

        snprintf(sz, sizeof(sz), "%d", s);
        memcpy(buf, sz+strlen(sz)-2, 2);
        return atoi(buf);
}

int Stb1(int s)
{
        char buf[8], sz[16];
        bzero(buf, 8);
        bzero(sz, 16);

        snprintf(sz, sizeof(sz), "%d", s);
        memcpy(buf, sz+strlen(sz)-3, 1);
        return atoi(buf);
}

int Sdb2(const char *s)
{
        if (s == NULL) return -1;
        
        char buf[8];
        bzero(buf, 8);
        //sprintf(buf, "%s", s[strlen(s)-2]);
        memcpy(buf, s+strlen(s)-2, 2);
        return atoi(buf);
}

int Stb2(const char *s)
{
        if (s == NULL) return -1;

        char buf[8];
        bzero(buf, 8);
        memcpy(buf, s+strlen(s)-3, 1);
        return atoi(buf);
}

int Sdb2_qqid(const char *s)
{
        if (s == NULL) return -1;
        int  dbIndex=0;  
	if (isDigitString(s))	
    {
	  dbIndex =  Sdb2(s);     
    }
	 else
    {
        string digest = getMd5(s);
        int len = digest.length();
        int h = 1000 + digest[len - 3] % 10 * 100 + digest[len - 2] % 10 * 10 + digest[len - 1] % 10;
        dbIndex = h % 100; //百位和十位
    }
	 return dbIndex;

}

int Stb2_qqid(const char *s)
{
        if (s == NULL) return -1;
        int tbIndex =0;
       if (isDigitString(s))	
    {
	  tbIndex =  Stb2(s);     
    }
	 else
    {
        string digest = getMd5(s);
        int len = digest.length();
        int h = 1000 + digest[len - 3] % 10 * 100 + digest[len - 2] % 10 * 10 + digest[len - 1] % 10;
         tbIndex = (h % 1000) / 100;//个位
    }
	 return tbIndex;
}

/**
* 按交易单号进行分表，交易单号规则是前10为spid 中间8位为时间的格式
*/
int Stb2_listid(const char *s)
{
        if (s == NULL) return -1;

        char buf[8];
        bzero(buf, 8);
        memcpy(buf, s+10, 6);
        return atoi(buf);
}



/**
 * Description: 得到当前时间字符串
 */
void GetTimeNow(char *str)
{
		if( NULL == str )
        return;
        
    time_t tt;
    struct tm stTm;

    tt = time(NULL);
    memset(&stTm, 0, sizeof(stTm));
    localtime_r(&tt, &stTm);
    snprintf(str, 20,
            "%04d-%02d-%02d %02d:%02d:%02d",
            stTm.tm_year + 1900, stTm.tm_mon + 1,
            stTm.tm_mday, stTm.tm_hour, stTm.tm_min, stTm.tm_sec);

        return;
}


/**
 * Description: 得到当前日期，格式yyyymmdd
 */
int GetDateToday()
{
        time_t tt;
        struct tm stTm;

        char  str[8+1];
        memset(str, 0, sizeof(str)-1);

        tt = time(NULL);
        memset(&stTm, 0, sizeof(stTm));
        localtime_r(&tt, &stTm);
        snprintf(str, sizeof(str), 
                "%04d%02d%02d",
                stTm.tm_year + 1900, stTm.tm_mon + 1,stTm.tm_mday);

        return atoi(str);
}

string GetDateTodayStr()
{
	time_t tt;
    struct tm stTm;

    char  str[11+1];
    memset(str, 0, sizeof(str));

    tt = time(NULL);
    memset(&stTm, 0, sizeof(stTm));
    localtime_r(&tt, &stTm);
    snprintf(str, sizeof(str), 
            "%04d-%02d-%02d",
            stTm.tm_year + 1900, stTm.tm_mon + 1,stTm.tm_mday);
	return string(str);
}
string getTime_yyyymm(int monOffset)
{
    time_t t  = time(NULL);
    struct  tm tm_now;
    localtime_r(&t, &tm_now);
    
    char szTmp[256];
    int year = tm_now.tm_year + 1900;
    int mon = tm_now.tm_mon + 1;

    int totalMons = year*12+mon+monOffset;
    year = (totalMons-1)/12;
    mon = totalMons - year*12;
    snprintf(szTmp, sizeof(szTmp), "%04d%02d",year, mon);
    return string(szTmp);
}

/**
 * Description: 得到当前时间，格式hh:mm:ss
 */
string GetTimeToday()
{
        time_t tt;
        struct tm stTm;

        char  str[8+1];
        memset(str, 0, sizeof(str)-1);

        tt = time(NULL);
        memset(&stTm, 0, sizeof(stTm));
        localtime_r(&tt, &stTm);
        snprintf(str, sizeof(str), 
                "%02d:%02d:%02d",
                stTm.tm_hour, stTm.tm_min, stTm.tm_sec);

        return string(str);
}

/**
 * Description:是否在时间区间内，格式%Y-%m-%d %H:%M:%S
 */
bool IsTimeLimt(string lower_time,string taller_time,string cur_time)
{
	time_t cur;
	struct tm lower_tm, taller_tm, cur_tm;
	if (cur_time.empty())
	{
		cur = time(NULL);
	}
	else
	{
		strptime(cur_time.c_str(), "%Y-%m-%d %H:%M:%S", &cur_tm);
		cur = mktime(&cur_tm);
	}

	strptime(lower_time.c_str(), "%Y-%m-%d %H:%M:%S", &lower_tm);
	strptime(taller_time.c_str(), "%Y-%m-%d %H:%M:%S", &taller_tm);

	return mktime(&lower_tm) <= cur && mktime(&taller_tm) >=cur;
		
}

string SetTodayTime(string time)
{
	string time_str = GetDateTodayStr();
	return time_str+' '+time;
}
	
void GetTimeColumn(int &year, int &month, int &day, int &hour, int &minute, int &second)
{
#define YEAR_BUF_LEN 64

    struct tm *timenow;
    time_t timet;
    char tmp_buf[YEAR_BUF_LEN + 1];

    timet = time(NULL);
    timenow = localtime(&timet);

    memset(tmp_buf, 0, YEAR_BUF_LEN + 1);
    strftime(tmp_buf, YEAR_BUF_LEN, "%Y", timenow);
    year = atoi(tmp_buf);

    memset(tmp_buf, 0, YEAR_BUF_LEN);
    strftime(tmp_buf, YEAR_BUF_LEN, "%m", timenow);
    month = atoi(tmp_buf);

    memset(tmp_buf, 0, YEAR_BUF_LEN);
    strftime(tmp_buf, YEAR_BUF_LEN, "%d", timenow);
    day = atoi(tmp_buf);

    memset(tmp_buf, 0, YEAR_BUF_LEN);
    strftime(tmp_buf, YEAR_BUF_LEN, "%H", timenow);
    hour = atoi(tmp_buf);

    memset(tmp_buf, 0, YEAR_BUF_LEN);
    strftime(tmp_buf, YEAR_BUF_LEN, "%M", timenow);
    minute = atoi(tmp_buf);

    memset(tmp_buf, 0, YEAR_BUF_LEN);
    strftime(tmp_buf, YEAR_BUF_LEN, "%S", timenow);
    second = atoi(tmp_buf);

    return;

#undef YEAR_BUF_LEN
}


/**
 * Description: 将时间转换成当天的时间格式
 * @input: hh:mm:ss
 * @output:  yyyy-mm-dd hh:mm:ss
 */
string toTodayTime(const char* hms)
{
        time_t tt;
        struct tm stTm;

        char str[20+1];
        memset(str, 0, sizeof(str)-1);

        tt = time(NULL);
        memset(&stTm, 0, sizeof(stTm));
        localtime_r(&tt, &stTm);
        snprintf(str, sizeof(str), 
                "%04d-%02d-%02d %s",
                stTm.tm_year + 1900, stTm.tm_mon + 1, stTm.tm_mday, hms);

        return str;
}

/* 获取当前时间的前一日日期 */
string getLastDate()
{

	time_t tt;
	struct tm *ptmnext;

    tt = time(NULL);
	tt -=  24 * 3600;//前一日

	ptmnext =  localtime(&tt);

	char szDateTime[20]= {0};
	snprintf(szDateTime, sizeof(szDateTime), "%04d%02d%02d", ptmnext->tm_year + 1900, ptmnext->tm_mon + 1, ptmnext->tm_mday);

	return szDateTime;
}

/**
* 转换时间格式
* input YYYY-MM-DD HH:MM:SS
* output YYYYMMDD
*/
string changeDatetime2Date(const char* date)
{
    int year, month, day, hour, minute, second;
    sscanf(date, "%04d-%02d-%02d %02d:%02d:%02d", &year, &month, &day, &hour, &minute, &second);
    char szTmp[256];
    snprintf(szTmp, sizeof(szTmp), "%04d%02d%02d",year,month,day);

    return szTmp;
}

/**
* 转换时间格式
* input YYYYMMDDHHMMSS
* output YYYY-MM-DD HH:MM:SS
*/
string changeDatetimeFormat( string date )
{
    int year, month, day, hour, minute, second;
    sscanf(date.c_str(), "%04d%02d%02d%02d%02d%02d", &year, &month, &day, &hour, &minute, &second);
    char szTmp[256];
    snprintf(szTmp, sizeof(szTmp), "%04d-%02d-%02d %02d:%02d:%02d",year,month,day,hour, minute, second);

    return szTmp;
}

/**
*转换日期格式
* type =1 :YYYYMMDD转YYYY-MM-DD， type=2 :YYYY-MM-DD转YYYYMMDD
*/
string changeDateFormat(string date ,int type)
{
    int year, month, day;
	char szTmp[256];
	if(1 == type)
	{
	    sscanf(date.c_str(), "%04d%02d%02d", &year, &month, &day);
	    snprintf(szTmp, sizeof(szTmp), "%04d-%02d-%02d",year,month,day );
	}
	else
	{
		sscanf(date.c_str(), "%04d-%02d-%02d", &year, &month, &day);
	    snprintf(szTmp, sizeof(szTmp), "%04d%02d%02d",year,month,day );
	}
    return szTmp;
}

/**
 * 将时间转换为系统时间
 * @input       time     YYYY-MM-DD HH:MM:SS
 * 对格式进行严格检查 gloriage 2011-01-27
 */
unsigned int toUnixTime(const char* time)
{
    // 取年、月、日段
    int year, month, day, hour, minute, second;
	char buf[5] = {0};

    if(strlen(time) != 19)
        return 0;

    if((time[4] != '-') || (time[7] != '-') || (time[10] != ' ') || (time[13] != ':') || (time[16] != ':'))
        return 0;

    buf[0] = time[0];
	buf[1] = time[1];
	buf[2] = time[2];
	buf[3] = time[3];
	buf[4] = '\0';
	year = atoi(buf);
	if(year < 2000 || year > 2038)
        return 0;

    buf[0] = time[5];
	buf[1] = time[6];
	buf[2] = '\0';
	month = atoi(buf);
	if(month < 1 || month > 12)
        return 0;

    buf[0] = time[8];
	buf[1] = time[9];
	buf[2] = '\0';
	day = atoi(buf);
	if(day < 1 || day > 31)
        return 0;

    buf[0] = time[11];
	buf[1] = time[12];
	buf[2] = '\0';
    if(!isNumString(buf))
        return 0;
	hour = atoi(buf);
	if(hour < 0 || hour > 23)
        return 0;

    buf[0] = time[14];
	buf[1] = time[15];
	buf[2] = '\0';
    if(!isNumString(buf))
        return 0;
	minute = atoi(buf);
	if(minute < 0 || minute > 59)
        return 0;

    buf[0] = time[17];
	buf[1] = time[18];
	buf[2] = '\0';
    if(!isNumString(buf))
        return 0;
	second = atoi(buf);
	if(second < 0 || second > 59)
        return 0;

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
    return  (unsigned int)mktime(&tm_date);
}

string toLocalTime(time_t t_time)
{
    struct tm tm_time;
    char buff[128];

    localtime_r(&t_time, &tm_time);

    memset(buff, 0, sizeof(buff));
    strftime(buff, sizeof(buff) - 1, "%Y-%m-%d %H:%M:%S", &tm_time);

    return buff;
}

/*
 * 将金额转化为元字符串
 */
string getAmountStr(LONG amount)
{
    char buff[128];

    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff) - 1, "%.2f", amount / 100.0);

    return buff;
}

/**
 * 计算有效截止期
 * @input:    string  YYYYMMDD
 * @output:   string  YYYYMMDD
 */
string addDays(const string& str, int days)
{
	struct tm tmdate;
	struct tm *ptmnext;
	time_t timep;

	int year, month, day;
	sscanf(str.c_str(),"%04d%02d%02d", &year, &month, &day);
	tmdate.tm_year = year - 1900;
    tmdate.tm_mon = month - 1;
    tmdate.tm_mday = day;
    tmdate.tm_hour = 0;
    tmdate.tm_min = 0;
    tmdate.tm_sec = 1;
    tmdate.tm_isdst = 0;

	timep = mktime(&tmdate);
	timep +=  24 * 3600 * days;

	ptmnext =  localtime(&timep);

	char szDateTime[20]= {0};
	snprintf(szDateTime, sizeof(szDateTime), "%04d%02d%02d", ptmnext->tm_year + 1900, ptmnext->tm_mon + 1, ptmnext->tm_mday);

	 return szDateTime;
}


/**
 * 对字符串进行SQL转义
 */
string escapeString(const string& str)
{
    static char szTmp[8192 + 1];
    memset(szTmp, 0, sizeof(szTmp));
    mysql_escape_string(szTmp, str.c_str(), str.length());

    return szTmp;
}

/**
 * 解密原始消息
 * @input    pRequest        消息请求句柄          
 * @output  szSpId            商户SPID
 *               szBuf              解密后的消息字符串
 */
void getDecodeMsg(TRPC_SVCINFO* pRequst, char* szBuf, char* szSpId) throw (CException)
{    
    // 返回结果信息
    ST_PUB_ANS stAns;
    memset(&stAns, 0 ,sizeof(stAns));

    // 临时变量
    char szEncodeMsg[MAX_MSG_LEN] = {0};
    
    // 获取消息原始加密串
    CUrlAnalyze::getParam((char*)(pRequst->idata), "sp_id", szSpId, MAX_SPID_LEN);
    CUrlAnalyze::getParam((char*)(pRequst->idata), "request_text", szEncodeMsg, MAX_MSG_LEN);

    // 解密消息数据
    decode(szSpId, szEncodeMsg, szBuf, stAns);
    if(stAns.iResult != 0)
    {
        throw CException(stAns.iResult, stAns.szErrInfo, __FILE__, __LINE__);
    }
}


string getMd5(const string &src_text)
{
    char md5_buf[32 + 1] = {0};
    memset(md5_buf, 0, sizeof(md5_buf));

    getMd5(src_text.c_str(), src_text.length(), md5_buf);

    return md5_buf;
}

/**
 * 从BILLNO SERVER获取订单号
 * @input:   strIP       BILLNO SERVER的ip
 *               iPort       BILLNO SERVER的端口
 *               iAppid     应用系统号
 * @return: 订单号
 */
unsigned int getBillno(const string& strIP, int iPort, int iAppid) throw (CException)
{
    unsigned int billno = 0;

    BillNO stBillno((char*)strIP.c_str(), iPort);

again:
    
    if(stBillno.getBillNO(iAppid, &billno) != 0) 
    {
        char szErrMsg[256] = {0};
        stBillno.getErrMsg(szErrMsg);
        throw CException(ERR_GET_BILLNO, szErrMsg, __FILE__, __LINE__);
    }

    // 判断billno 的最后六位是否全为0， 为0则重新获取
    if (billno % 1000000 == 0) goto again;

    return billno;
}

/**
 * 从生成子账户提现单号 8位日期+10序列号
 * @return: 订单号
 */

string genSubaccDrawid() throw (CException)
{
	char  szListid[MAX_LISTID_LENGTH+1] = {0};

    // 取订单序列号
    unsigned int billno = getBillno(gPtrConfig->m_BillnoCfg.host, gPtrConfig->m_BillnoCfg.port, gPtrConfig->m_BillnoCfg.appid);

    // 取当前时间
    time_t tt = time(NULL);
    struct tm tm1;
    localtime_r(&tt, &tm1);

    int date = (tm1.tm_year + 1900) * 10000 + (tm1.tm_mon + 1) * 100 + tm1.tm_mday;

    // 生成提现单号: 8位日期+10位序列号
    snprintf(szListid, sizeof(szListid),  "%d%.10u", date, billno);
	return szListid;
}



/**
 * 调用差错服务器
 * @input:  rqst   middle消息结构
 *              pLog 日志文件句柄
 * @note: 
 *          1、该接口不抛出异常
 *          2、该接口的调用和返回记录单独日志文件
 */
void callErrorRpc(TRPC_SVCINFO* rqst, CftLog* pLog)
{        
    // 将错误消息发给差错处理
    int iRet = trpc_error_call(rqst->idata, rqst->ilen, 2);

    // 只记录首次发给差错的日志
    if(trpc_packet_type(rqst) != TRPC_CALL_TYPE_ERROR)
    {
        // 成功写正常日志，失败写错误日志
        if(iRet == 0)
        {
            pLog->normal("%s|%d", rqst->idata, iRet);
        }
        else
        {
            pLog->error("%s|%d", rqst->idata, iRet);
        }
    }
}


/**
 * 是否来自差错
 * @input:  rqst   middle消息结构
 * @return: true: 来自差错，false: 来自业务
 */
bool isTrpcErrMsg(TRPC_SVCINFO* rqst)
{
    return (trpc_packet_type(rqst) == TRPC_CALL_TYPE_ERROR);
}

int createSubaccUser(TRPC_SVCINFO* m_request, const string &spid, const string &uin , const string &client_ip,int subacc_curType)
{
	int iResult =-1;
	try
	{
		iResult = SubaccCreateUser(gPtrSubaccRpc, spid, uin, client_ip,subacc_curType);
	}
	catch(CException& e)
	{
		
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		// 发消息给差错
		callErrorRpc(m_request, gPtrSysLog);
		if(ERR_TYPE_MSG)
		{
			throw;//来自差错补单的直接抛出异常，阻止后面继续执行
		}
		
	}

	if(0 != iResult && ERR_SUBACC_EXIST != iResult)
	{
		//60120105:		表示用户已存在
		throw CUnknowException(ERR_FAIL_CALL_TRANS, "create user call transaction failed!", __FILE__, __LINE__);
	}

    return iResult;
}


/**
*在子账户中创建账户
*/
int SubaccCreateUser(CRpcWrapper* pRpc, const string &spid, const string &uin , const string &client_ip,int subacc_curType)
{
	char szMsg[MAX_MSG_LEN] = {0};
    char szBuf[MAX_MSG_LEN] = {0};
    int iResult = -1, oLen=sizeof(szBuf);
    char szResInfo[256] = {0};

    int subacc_cur_type = subacc_curType;
    if (subacc_cur_type < 0)
   {
       subacc_cur_type = querySubaccCurtype(gPtrFundDB, spid);
   }

    CUrlAnalyze::setParam(szMsg, "spid", spid.c_str(), true);
    CUrlAnalyze::setParam(szMsg, "uin", uin.c_str()); 
    CUrlAnalyze::setParam(szMsg, "uin_flag", 1);
    CUrlAnalyze::setParam(szMsg, "action", 1);
    CUrlAnalyze::setParam(szMsg, "cur_type", subacc_cur_type); 
    CUrlAnalyze::setParam(szMsg, "client_ip", client_ip.c_str());
    //CUrlAnalyze::setParam(szMsg, "desc", m_params.getString("card_no").c_str());

	stringstream ss;
    char buff[128] = {0};
    // 按照spid|uin|cur_type生成原串
    ss << spid << ":" ;
    ss << uin << ":" ;
    ss << subacc_cur_type;
    getMd5(ss.str().c_str(), ss.str().size(), buff);

    CUrlAnalyze::setParam(szMsg, "call_token", buff);


    ST_PUB_ANS stAns;
    memset(&stAns, 0, sizeof(ST_PUB_ANS));
    char szEncode[MAX_MSG_LEN + 1] = {0};
    encode(spid.c_str(), szMsg, szEncode, stAns);
    if (stAns.iResult != 0)
    {
        throw EXCEPTION(stAns.iResult, stAns.szErrInfo);
    }

    // 调用UI接口
    memset(szMsg, 0, sizeof(szMsg));
    snprintf(szMsg, sizeof(szMsg), "%s=%s&%s=%s&%s=%s",
             "sp_id", spid.c_str(), "request_text", szEncode, "MSG_NO", MSG_NO);

	//网络异常重试
	try
	{
		//重入是通过result返回，而不是异常
    	pRpc->excute("subacc_mgr_service", szMsg, strlen(szMsg), szBuf, oLen);
	}
	catch(CException& e)
	{
		//59012004,read: Connection reset by peer: errno==104, Connection reset by peer 线上会返回这个错误，导致子账户开户失败
		//异常重试两次
		try
		{
			
			usleep(100); //对方有问题就睡眠100ms等待
	    	pRpc->excute("subacc_mgr_service", szMsg, strlen(szMsg), szBuf, oLen);
		}
		catch(CException& e)
		{	
			usleep(100); //对方有问题就睡眠100ms等待
			pRpc->excute("subacc_mgr_service", szMsg, strlen(szMsg), szBuf, oLen);
		}
	}

    // 取返回结果
    CUrlAnalyze::getParam(szBuf, "result", &iResult);
    CUrlAnalyze::getParam(szBuf, "res_info", szResInfo, sizeof(szResInfo));

	return iResult;
}

/**
*input type  1. 申购; 2. 收益
*/
int SubaccSave(CRpcWrapper* pRpc, const string &spid, const string &uin , const string &client_ip, const string &transaction_id, const LONG &total_fee, const string &desc, const string & acc_time,int type,int subacc_curType)
{	
	char szMsg[MAX_MSG_LEN] = {0};
    char szBuf[MAX_MSG_LEN] = {0};
    int iResult = -1, oLen=sizeof(szBuf);
    char szResInfo[256] = {0};

    int sub_acc_cur_type = subacc_curType;
    if (sub_acc_cur_type < 0)
   {
       sub_acc_cur_type = querySubaccCurtype(gPtrFundDB, spid);
   }

	SET_PARAM(szMsg, "transaction_id", transaction_id.c_str(), true);
    SET_PARAM(szMsg, "spid", spid);
    SET_PARAM(szMsg, "uin", uin); 
    SET_PARAM(szMsg, "cur_type", sub_acc_cur_type); 
	SET_PARAM(szMsg, "total_fee", total_fee); 
    SET_PARAM(szMsg, "client_ip", client_ip);
    SET_PARAM(szMsg, "desc", desc);
	SET_PARAM(szMsg, "client_ip", client_ip);
	SET_PARAM(szMsg, "acc_time", acc_time);
	SET_PARAM(szMsg, "type", type);

	stringstream ss;
    char buff[128] = {0};
    // 按照spid|uin|cur_type生成原串
    ss << transaction_id << ":" ;
    ss << uin << ":" ;
	ss << total_fee ;
    getMd5(ss.str().c_str(), ss.str().size(), buff);

    CUrlAnalyze::setParam(szMsg, "call_token", buff);


    ST_PUB_ANS stAns;
    memset(&stAns, 0, sizeof(ST_PUB_ANS));
    char szEncode[MAX_MSG_LEN + 1] = {0};
    encode(spid.c_str(), szMsg, szEncode, stAns);
    if (stAns.iResult != 0)
    {
        throw EXCEPTION(stAns.iResult, stAns.szErrInfo);
    }

    // 调用UI接口
    memset(szMsg, 0, sizeof(szMsg));
    snprintf(szMsg, sizeof(szMsg), "%s=%s&%s=%s&%s=%s",
             "sp_id", spid.c_str(), "request_text", szEncode, "MSG_NO", MSG_NO);

	//网络异常抛给差错
	try
	{
		//重入会返回result=0&res_info=ok
    	pRpc->excute("subacc_ba_save_service", szMsg, strlen(szMsg), szBuf, oLen);
	}
	catch(CException& e)
	{
		//59012004,read: Connection reset by peer: errno==104, Connection reset by peer 线上会返回这个错误，导致子账户充值失败
		//充值报错重试两次
		try
		{
			
			usleep(50); //对方有问题就睡眠50ms等待
	    	pRpc->excute("subacc_ba_save_service", szMsg, strlen(szMsg), szBuf, oLen);
		}
		catch(CException& e)
		{	
			usleep(50); //对方有问题就睡眠50ms等待
			pRpc->excute("subacc_ba_save_service", szMsg, strlen(szMsg), szBuf, oLen);
		}

	}

    // 取返回结果
    CUrlAnalyze::getParam(szBuf, "result", &iResult);
    CUrlAnalyze::getParam(szBuf, "res_info", szResInfo, sizeof(szResInfo));
	if(iResult != 0 && iResult != ERR_SUBACC_NOT_EXIST)
	{
		//1012 未开通子账户
		throw CException(iResult, szResInfo, __FILE__, __LINE__);
	}

	return iResult;
}
int SubaccDraw(CRpcWrapper* pRpc, const string &spid, const string &uin , const string &client_ip, const string &transaction_id, const LONG &total_fee, const string & acc_time,const string&controllist)
{
	char szMsg[MAX_MSG_LEN] = {0};
    char szBuf[MAX_MSG_LEN] = {0};
    int iResult = -1, oLen=sizeof(szBuf);
    char szResInfo[256] = {0};

	SET_PARAM(szMsg, "transaction_id", transaction_id, true);
    SET_PARAM(szMsg, "spid", spid);
    SET_PARAM(szMsg, "uin", uin); 
    SET_PARAM(szMsg, "cur_type", querySubaccCurtype(gPtrFundDB, spid)); 
	SET_PARAM(szMsg, "total_fee", total_fee); 
    SET_PARAM(szMsg, "client_ip", client_ip);
	SET_PARAM(szMsg, "acc_time", acc_time);
    if (controllist != "")
    {
        CUrlAnalyze::setParam(szMsg, "control_num", total_fee);
        CUrlAnalyze::setParam(szMsg, "control_type", 6);
        CUrlAnalyze::setParam(szMsg, "control_list", controllist.c_str());
    }
	
    //CUrlAnalyze::setParam(szMsg, "desc", desc);

	stringstream ss;
    char buff[128] = {0};
    // 按照spid|uin|cur_type生成原串
    ss << transaction_id << ":" ;
    ss << uin << ":" ;
	ss << total_fee ;
    getMd5(ss.str().c_str(), ss.str().size(), buff);

    CUrlAnalyze::setParam(szMsg, "call_token", buff);


    ST_PUB_ANS stAns;
    memset(&stAns, 0, sizeof(ST_PUB_ANS));
    char szEncode[MAX_MSG_LEN + 1] = {0};
    encode(spid.c_str(), szMsg, szEncode, stAns);
    if (stAns.iResult != 0)
    {
        throw EXCEPTION(stAns.iResult, stAns.szErrInfo);
    }

    // 调用UI接口
    memset(szMsg, 0, sizeof(szMsg));
    snprintf(szMsg, sizeof(szMsg), "%s=%s&%s=%s&%s=%s",
             "sp_id", spid.c_str(), "request_text", szEncode, "MSG_NO", MSG_NO);

	//网络异常抛给差错
	try
	{
		//重入会返回result=0&res_info=ok
    	pRpc->excute("subacc_ba_fetch_service", szMsg, strlen(szMsg), szBuf, oLen);
	}
	catch(CException& e)
	{
		//59012004,read: Connection reset by peer: errno==104, Connection reset by peer 线上会返回这个错误，导致子账户充值失败
		//充值报错重试两次
		try
		{
			
			usleep(100); //对方有问题就睡眠100ms等待
	    	pRpc->excute("subacc_ba_fetch_service", szMsg, strlen(szMsg), szBuf, oLen);
		}
		catch(CException& e)
		{	
			usleep(100); //对方有问题就睡眠100ms等待
			pRpc->excute("subacc_ba_fetch_service", szMsg, strlen(szMsg), szBuf, oLen);
		}

	}

    // 取返回结果
    CUrlAnalyze::getParam(szBuf, "result", &iResult);
    CUrlAnalyze::getParam(szBuf, "res_info", szResInfo, sizeof(szResInfo));

	if(iResult != 0)
	{
		throw CException(iResult, szResInfo, __FILE__, __LINE__);
	}

	return iResult;
}


int SubaccFetchReq(CRpcWrapper* pRpc, const string &spid, const string &uin , const string &client_ip, const string &transaction_id, const LONG &total_fee, const string & acc_time,int subacc_curType)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szBuf[MAX_MSG_LEN] = {0};
    int iResult = -1, oLen=sizeof(szBuf);
    char szResInfo[256] = {0};

    int sub_acc_cur_type = subacc_curType;
    if (sub_acc_cur_type<0)
    {
        sub_acc_cur_type = querySubaccCurtype(gPtrFundDB, spid);
    }

    SET_PARAM(szMsg, "transaction_id", transaction_id, true);
    SET_PARAM(szMsg, "spid", spid);
    SET_PARAM(szMsg, "uin", uin); 
    SET_PARAM(szMsg, "cur_type", sub_acc_cur_type); 
    SET_PARAM(szMsg, "total_fee", total_fee); 
    SET_PARAM(szMsg, "client_ip", client_ip);
    SET_PARAM(szMsg, "acc_time", acc_time);
	

    stringstream ss;
    char buff[128] = {0};

    ss << transaction_id << ":" ;
    ss << uin << ":" ;
    ss << sub_acc_cur_type << ":" ;
    ss << total_fee ;
    getMd5(ss.str().c_str(), ss.str().size(), buff);

    CUrlAnalyze::setParam(szMsg, "call_token", buff);


    ST_PUB_ANS stAns;
    memset(&stAns, 0, sizeof(ST_PUB_ANS));
    char szEncode[MAX_MSG_LEN + 1] = {0};
    encode(spid.c_str(), szMsg, szEncode, stAns);
    if (stAns.iResult != 0)
    {
        throw EXCEPTION(stAns.iResult, stAns.szErrInfo);
    }

    // 调用UI接口
    memset(szMsg, 0, sizeof(szMsg));
    snprintf(szMsg, sizeof(szMsg), "%s=%s&%s=%s&%s=%s",
             "sp_id", spid.c_str(), "request_text", szEncode, "MSG_NO", MSG_NO);

    //网络异常抛给差错
    try
    {
        //重入会返回result=0&res_info=ok
        pRpc->excute("subacc_ba_fetchreq_service", szMsg, strlen(szMsg), szBuf, oLen);
    }
    catch(CException& e)
    {
        //59012004,read: Connection reset by peer: errno==104, Connection reset by peer 线上会返回这个错误，导致子账户充值失败
        //充值报错重试两次
        try
        {
            usleep(100); //对方有问题就睡眠100ms等待
            pRpc->excute("subacc_ba_fetchreq_service", szMsg, strlen(szMsg), szBuf, oLen);
        }
        catch(CException& e)
        {	
            usleep(100); //对方有问题就睡眠100ms等待
            pRpc->excute("subacc_ba_fetchreq_service", szMsg, strlen(szMsg), szBuf, oLen);
        }

    }

    // 取返回结果
    CUrlAnalyze::getParam(szBuf, "result", &iResult);
    CUrlAnalyze::getParam(szBuf, "res_info", szResInfo, sizeof(szResInfo));

    if(iResult != 0)
    {
        throw CException(iResult, szResInfo, __FILE__, __LINE__);
    }

    return iResult;
}

int SubaccFetchResult(CRpcWrapper* pRpc, const string &spid, const string &uin , const string &client_ip, const string &transaction_id, const LONG &total_fee, const string & acc_time,int result_sign,int subacc_curType)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szBuf[MAX_MSG_LEN] = {0};
    int iResult = -1, oLen=sizeof(szBuf);
    char szResInfo[256] = {0};

    int sub_acc_cur_type = subacc_curType;
    if (sub_acc_cur_type<0)
    {
        sub_acc_cur_type = querySubaccCurtype(gPtrFundDB, spid);
    }

    SET_PARAM(szMsg, "transaction_id", transaction_id, true);
    SET_PARAM(szMsg, "spid", spid);
    SET_PARAM(szMsg, "uin", uin); 
    SET_PARAM(szMsg, "cur_type", sub_acc_cur_type); 
    SET_PARAM(szMsg, "total_fee", total_fee); 
    SET_PARAM(szMsg, "client_ip", client_ip);
    SET_PARAM(szMsg, "acc_time", acc_time);
    
    CUrlAnalyze::setParam(szMsg, "result_sign", result_sign);

    stringstream ss;
    char buff[128] = {0};
    //(transaction_id:uin:total_fee:cur_type:result_sign)
    ss << transaction_id << ":" ;
    ss << uin << ":" ;
    ss << sub_acc_cur_type << ":" ;
    ss << total_fee << ":" ;
    ss << result_sign  ;
    getMd5(ss.str().c_str(), ss.str().size(), buff);

    CUrlAnalyze::setParam(szMsg, "call_token", buff);


    ST_PUB_ANS stAns;
    memset(&stAns, 0, sizeof(ST_PUB_ANS));
    char szEncode[MAX_MSG_LEN + 1] = {0};
    encode(spid.c_str(), szMsg, szEncode, stAns);
    if (stAns.iResult != 0)
    {
        throw EXCEPTION(stAns.iResult, stAns.szErrInfo);
    }

    // 调用UI接口
    memset(szMsg, 0, sizeof(szMsg));
    snprintf(szMsg, sizeof(szMsg), "%s=%s&%s=%s&%s=%s",
             "sp_id", spid.c_str(), "request_text", szEncode, "MSG_NO", MSG_NO);

    //网络异常抛给差错
    try
    {
        //重入会返回result=0&res_info=ok
        pRpc->excute("subacc_ba_fetchresult_service", szMsg, strlen(szMsg), szBuf, oLen);
    }
    catch(CException& e)
    {
        //59012004,read: Connection reset by peer: errno==104, Connection reset by peer 线上会返回这个错误，导致子账户充值失败
        //充值报错重试两次
        try
        {
            usleep(100); //对方有问题就睡眠100ms等待
            pRpc->excute("subacc_ba_fetchresult_service", szMsg, strlen(szMsg), szBuf, oLen);
        }
        catch(CException& e)
        {	
            usleep(100); //对方有问题就睡眠100ms等待
            pRpc->excute("subacc_ba_fetchresult_service", szMsg, strlen(szMsg), szBuf, oLen);
        }

    }

    // 取返回结果
    CUrlAnalyze::getParam(szBuf, "result", &iResult);
    CUrlAnalyze::getParam(szBuf, "res_info", szResInfo, sizeof(szResInfo));

    if(iResult != 0)
    {
        throw CException(iResult, szResInfo, __FILE__, __LINE__);
    }

    return iResult;
}


int SubaccFreeze(CRpcWrapper* pRpc, const string &spid, const string &uin , const string &client_ip, const string &transaction_id, const LONG &total_fee, const string & acc_time,const string &desc)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szBuf[MAX_MSG_LEN] = {0};
    int iResult = -1, oLen=sizeof(szBuf);
    char szResInfo[256] = {0};
    int cur_type = querySubaccCurtype(gPtrFundDB, spid);
    CUrlAnalyze::setParam(szMsg, "transaction_id", transaction_id.c_str(),true);
    CUrlAnalyze::setParam(szMsg, "freeze_listid", transaction_id.c_str());
    CUrlAnalyze::setParam(szMsg, "spid", spid.c_str());
    CUrlAnalyze::setParam(szMsg, "uin", uin.c_str()); 
    CUrlAnalyze::setParam(szMsg, "cur_type", cur_type); 
    CUrlAnalyze::setParam(szMsg, "freeze_num", total_fee); 
    CUrlAnalyze::setParam(szMsg, "freeze_ip", client_ip.c_str());
    CUrlAnalyze::setParam(szMsg, "acc_time", acc_time.c_str());
    CUrlAnalyze::setParam(szMsg, "freeze_memo", desc.c_str());

    stringstream ss;
    char buff[128] = {0};
    // 按照Md5(trans_id:uin:cur_type:freeze_num)
    ss << transaction_id << ":" ;
    ss << uin << ":" ;
    ss << cur_type<< ":" ;
    ss << total_fee  ;

    TRACE_DEBUG("***%s****", ss.str().c_str());
    
    getMd5(ss.str().c_str(), ss.str().size(), buff);
    CUrlAnalyze::setParam(szMsg, "token", buff);
    CUrlAnalyze::setParam(szMsg, "vali_state", 2);// 是否验证用户状态  2:不验证
    CUrlAnalyze::setParam(szMsg, "source", 6);
    CUrlAnalyze::setParam(szMsg, "reason", 6);

    ST_PUB_ANS stAns;
    memset(&stAns, 0, sizeof(ST_PUB_ANS));
    char szEncode[MAX_MSG_LEN + 1] = {0};
    encode(spid.c_str(), szMsg, szEncode, stAns);
    if (stAns.iResult != 0)
    {
        throw EXCEPTION(stAns.iResult, stAns.szErrInfo);
    }

    // 调用UI接口
    memset(szMsg, 0, sizeof(szMsg));
    snprintf(szMsg, sizeof(szMsg), "%s=%s&%s=%s&%s=%s","sp_id", spid.c_str(), "request_text", szEncode, "MSG_NO", MSG_NO);


    try
    {
        //重入会返回result=0&res_info=ok
        pRpc->excute("subacc_ba_freeze_service", szMsg, strlen(szMsg), szBuf, oLen);
    }
    catch(CException& e)
    {
        //59012004,read: Connection reset by peer: errno==104, Connection reset by peer 线上会返回这个错误
        //重试两次
        try
        {
            usleep(100); //对方有问题就睡眠100ms等待
            pRpc->excute("subacc_ba_freeze_service", szMsg, strlen(szMsg), szBuf, oLen);
        }
        catch(CException& e)
        {	
            usleep(100); //对方有问题就睡眠100ms等待
            pRpc->excute("subacc_ba_freeze_service", szMsg, strlen(szMsg), szBuf, oLen);
        }
    }

    // 取返回结果
    CUrlAnalyze::getParam(szBuf, "result", &iResult);
    CUrlAnalyze::getParam(szBuf, "res_info", szResInfo, sizeof(szResInfo));

    if(iResult != 0)
    {
        throw CException(iResult, szResInfo, __FILE__, __LINE__);
    }

    
    return 0;
}

int SubaccUnFreeze(CRpcWrapper* pRpc, const string &spid, const string &uin , const string &client_ip, const string &unfreeze_id,const string &freeze_id, const LONG &total_fee, const string & acc_time,const string &control_list,const LONG &control_fee,const string & desc)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szBuf[MAX_MSG_LEN] = {0};
    int iResult = -1, oLen=sizeof(szBuf);
    char szResInfo[256] = {0};
    int cur_type = querySubaccCurtype(gPtrFundDB, spid);
    CUrlAnalyze::setParam(szMsg, "unfreeze_serialno", unfreeze_id.c_str(), true);
    CUrlAnalyze::setParam(szMsg, "transaction_id", freeze_id.c_str());
    CUrlAnalyze::setParam(szMsg, "freeze_listid", freeze_id.c_str());
    CUrlAnalyze::setParam(szMsg, "spid", spid.c_str());
    CUrlAnalyze::setParam(szMsg, "uin", uin.c_str()); 
    CUrlAnalyze::setParam(szMsg, "cur_type", cur_type); 
    CUrlAnalyze::setParam(szMsg, "unfreeze_num", total_fee); 
    CUrlAnalyze::setParam(szMsg, "client_ip", client_ip.c_str());
    CUrlAnalyze::setParam(szMsg, "unfreeze_ip", client_ip.c_str());
    CUrlAnalyze::setParam(szMsg, "acc_time", acc_time.c_str());
    CUrlAnalyze::setParam(szMsg, "unfreeze_memo", desc.c_str());

    stringstream ss;
    char buff[128] = {0};
    // 按照Md5(trans_id:uin:cur_type:freeze_num)
    ss << freeze_id << ":" ;
    ss << uin << ":" ;
    ss << cur_type<< ":" ;
    ss << total_fee  ;
    getMd5(ss.str().c_str(), ss.str().size(), buff);
    CUrlAnalyze::setParam(szMsg, "token", buff);
    CUrlAnalyze::setParam(szMsg, "vali_state", 2);
    CUrlAnalyze::setParam(szMsg, "source", 6);

    if (control_list.length() > 0)
    {
        CUrlAnalyze::setParam(szMsg, "control_num", control_fee);
        CUrlAnalyze::setParam(szMsg, "control_type", 6);
        CUrlAnalyze::setParam(szMsg, "control_list", control_list.c_str());
    }

    ST_PUB_ANS stAns;
    memset(&stAns, 0, sizeof(ST_PUB_ANS));
    char szEncode[MAX_MSG_LEN + 1] = {0};
    encode(spid.c_str(), szMsg, szEncode, stAns);
    if (stAns.iResult != 0)
    {
        throw EXCEPTION(stAns.iResult, stAns.szErrInfo);
    }

    // 调用UI接口
    memset(szMsg, 0, sizeof(szMsg));
    snprintf(szMsg, sizeof(szMsg), "%s=%s&%s=%s&%s=%s","sp_id", spid.c_str(), "request_text", szEncode, "MSG_NO", MSG_NO);


    try
    {
        //重入会返回result=0&res_info=ok
        pRpc->excute("subacc_ba_unfreeze_service", szMsg, strlen(szMsg), szBuf, oLen);
    }
    catch(CException& e)
    {
        //59012004,read: Connection reset by peer: errno==104, Connection reset by peer 线上会返回这个错误
        //报错重试两次
        try
        {
            usleep(100); //对方有问题就睡眠100ms等待
            pRpc->excute("subacc_ba_unfreeze_service", szMsg, strlen(szMsg), szBuf, oLen);
        }
        catch(CException& e)
        {	
            usleep(100); //对方有问题就睡眠100ms等待
            pRpc->excute("subacc_ba_unfreeze_service", szMsg, strlen(szMsg), szBuf, oLen);
        }
    }

    // 取返回结果
    CUrlAnalyze::getParam(szBuf, "result", &iResult);
    CUrlAnalyze::getParam(szBuf, "res_info", szResInfo, sizeof(szResInfo));

    if(iResult != 0)
    {  
        //alert(iResult, (string("子账户解冻失败单号:")+unfreeze_id+"错误码原因"+szResInfo).c_str());
        throw CException(iResult, szResInfo, __FILE__, __LINE__);
    }

    return 0;
}

/**
 * 将15位身份证号码转换成18位
 */
string TransIdentityNumber(const char* cre_id) throw (CException)
{
    char  szCreId18[18+1];
    int aiBitPower[17] = {7, 9, 10, 5, 8, 4, 2, 1, 6, 3, 7, 9, 10, 5, 8, 4, 2};
    char achCheck[11] = {'1', '0', 'X', '9', '8', '7', '6', '5', '4', '3', '2'};
    int iSum = 0;
    char  chCheck;
    char bit[2];

    memset(szCreId18, 0, sizeof(szCreId18));

    if(18 == strlen(cre_id))
    {
        // 18位身份证号码，如果最后一位是'X'，统一用大写
        if('x' == cre_id[17])
        {
            strncpy(szCreId18, cre_id, 17);
            szCreId18[17] = 'X';
        }
        // 18位身份证号码，原样返回
        else
        {
            strncpy(szCreId18, cre_id, 18);
        }
    }
    else if(15 == strlen(cre_id))
    {
        strncpy(szCreId18, cre_id, 6);
        strncpy(szCreId18+6, "19", 2);
        strncpy(szCreId18+8, cre_id+6, 9);

        // 计算末尾的校验码
        for(int index=0; index<17; index++)
        {
            bit[0] = szCreId18[index];
            bit[1] = '\0';
            iSum += aiBitPower[index] * atoi(bit);
        }
        chCheck = achCheck[iSum % 11];

        memcpy(szCreId18+17, &chCheck, 1);
    }
    else
    {
        throw CException(ERR_BAD_PARAM, "identity number length is not 15 or 18! ", __FILE__, __LINE__);
    }

    return szCreId18;
}


/**
 * 生成trade_id，trade_id = 8位日期(yyyymmdd)+9位序列号
 */
string GenerateTradeid(int uid)
{
    char  trade_id[MAX_LISTID_LENGTH+1] = {0};

    // 取billno
    unsigned int billno = getBillno(gPtrConfig->m_BillnoCfg.host, gPtrConfig->m_BillnoCfg.port, gPtrConfig->m_BillnoCfg.appid);

    // 取当前时间
    time_t tt = time(NULL);
    struct tm tm1;
    localtime_r(&tt, &tm1);

    int date = (tm1.tm_year + 1900) * 10000 + (tm1.tm_mon + 1) * 100 + tm1.tm_mday;

    // 生成trade_id: 8位日期(yyyymmdd)+9位序列号
    snprintf(trade_id, sizeof(trade_id),  "%d%.9u", date, billno);
	return trade_id;
}

/**
 * 从tradeid中获取uid，trade_id = 10位uid + 4位用uid生成的随机码
 */
string getUidFromTradeid(string tradeid)
{
	return tradeid.substr(0, 10);
}

/**
 * 从交易单号中得到基金交易日期
 */
string GetTradeDateFromListid(const char* listid)
{
    char szTradeDate[8+1] = {0};

    strncpy(szTradeDate, listid+10, 8);
    szTradeDate[8] = '\0';

    return szTradeDate;
}

int QueryFetchType(CMySQL* mysql, const string &Fcft_fetch_no)
{
	stringstream ss_sql;
    MYSQL_RES* result = NULL;
    MYSQL_ROW  row;
    int retNums=0;

	//要从Fcft_fetch_no提取日期(Fcft_fetch_no.substr(3,6))，所以长度要大于10
	if (Fcft_fetch_no.length() != 21)
	{
		TRACE_DEBUG("[Fcft_fetch_no不是标准长度]");
		return 0;
	}

    int date_yyyy = atoi(Fcft_fetch_no.substr(3,4).c_str());
    int date_mm = atoi(Fcft_fetch_no.substr(7,2).c_str());
    if (date_yyyy<2013 || date_mm>12 || date_mm<=0)
    {
    	//非标准格式的fetch_no不检查，默认当普通赎回。一般为活动请求
       	return 0;
    }
    ss_sql << "select Fstandby2 ";
    ss_sql << " from fund_db.t_fetch_list_" << Fcft_fetch_no.substr(3,6)<<" where Fcft_fetch_no = '" << Fcft_fetch_no<<"'";

	TRACE_DEBUG("[SQL:%s]",ss_sql.str().c_str());
	mysql->Query(ss_sql.str().c_str(), ss_sql.str().size());

    result = mysql->FetchResult();
    int rownum = mysql_num_rows(result);
    if (0 == rownum)
    {
        //没查询到返回0，可能有的流程会先调fund deal再调fund fetch
        //強赎一定是先调fund fetch
        mysql_free_result(result);
        return 0;
    }
    if (rownum > 1)
    {
        mysql_free_result(result);
        throw CException(ERR_UNIQUE_MULTIROWS, "query fund fetch list by cft_fetch_no get multiple rows! ", __FILE__, __LINE__);
    }

    row = mysql_fetch_row(result);
    retNums = atoi(ValiStr(row[0]));
    mysql_free_result(result);
	return retNums;
}

bool QueryFundBind(CMySQL* mysql, const string &cond, ST_FUND_BIND* pstRecord)
{
    int iRet = 0;
    stringstream ss_sql;
    MYSQL_RES* result = NULL;
    MYSQL_ROW  row;

    ss_sql << "select Fcre_type, Ftrade_id, Fcre_id, Fqqid, Fuid, Ftrue_name,";
    ss_sql << " Fcre_id_orig, Fphone, Fmobile, Fstate, Flstate,Fopenid,Facct_type,Fchannel_id,Fstandby1, ";
    ss_sql << " Fcreate_time,Fstandby2,Fstandby4,Fstandby3,Fstandby12, Fstandby5, Fstandby11 ";
    ss_sql << " from fund_db.t_fund_bind where " << cond;

    mysql->Query(ss_sql.str().c_str(), ss_sql.str().size());

    result = mysql->FetchResult();
    int rownum = mysql_num_rows(result);
    if (0 == rownum)
    {
        mysql_free_result(result);
        return false;
    }
    if (rownum > 1)
    {
        mysql_free_result(result);
        throw CException(ERR_UNIQUE_MULTIROWS, "query fund bind by creid get multiple rows! ", __FILE__, __LINE__);
    }

    row = mysql_fetch_row(result);

    pstRecord->Fcre_type = atoi(ValiStr(row[0]));
    strncpy(pstRecord->Ftrade_id, ValiStr(row[1]), sizeof(pstRecord->Ftrade_id)-1);
    
    //长度为DB_DECODE_CRE_ID_LENGTH表示加密后的数据才需解密，其余直接返回DB数据
    //由于Fcre_id字段外部使用场景很少，故解码错误的异常暂时只记录错误，不抛异常
    if( DB_DECODE_CRE_ID_MAX_LENGTH < strlen(row[2]) ){
        string strDeTo;
        TRACE_DEBUG("lct_decode cre_id, trade_id[%s] db_cre_id[%s]", row[1],row[2]);
        if((iRet = lct_decode(row[1], row[2], strDeTo))){
            gPtrAppLog->error("[%s][%d] lct_decode Fcre_id error! lct_decode Ftrade_id=%s db_Fcre_id=%s iRet=%d", 
                                __FILE__,__LINE__, row[1],row[2],iRet);
        }        
        strncpy(pstRecord->Fcre_id, ValiStr((char*)strDeTo.c_str()), sizeof(pstRecord->Fcre_id)-1);
    }else{
        strncpy(pstRecord->Fcre_id, ValiStr(row[2]), sizeof(pstRecord->Fcre_id)-1);
    }
    
    //长度为DB_DECODE_CRE_ID_LENGTH表示加密后的数据才需解密，其余直接返回DB数据
    //由于Fcre_id字段外部使用场景很少，故解码错误的异常暂时只记录错误，不抛异常
    if( DB_DECODE_CRE_ID_MAX_LENGTH < strlen(row[6]) ){
        string strDeTo;
        TRACE_DEBUG("lct_decode cre_id, trade_id[%s] db_cre_id_orig[%s]", row[1],row[6]);
        if((iRet = lct_decode(row[1], row[6], strDeTo))){
            gPtrAppLog->error("[%s][%d] lct_decode Fcre_id_orig error! lct_decode Ftrade_id=%s db_Fcre_id_orig=%s iRet=%d", 
                                __FILE__,__LINE__, row[1],row[6],iRet);
        }        
        strncpy(pstRecord->Fcre_id_orig, ValiStr((char*)strDeTo.c_str()), sizeof(pstRecord->Fcre_id_orig)-1);
    }else{
        strncpy(pstRecord->Fcre_id_orig, ValiStr(row[6]), sizeof(pstRecord->Fcre_id_orig)-1);
    }
    
    //长度大于DB_DECODE_CRE_ID_MAX_LENGTH表示加密后的数据才需解密，否则直接返回DB明文数据
    //由于Fphone字段外部使用场景很少，故解码错误的异常暂时只记录错误，不抛异常
    if( DB_DECODE_PHONE_MAX_LENGTH < strlen(row[7]) ){
        string strDeTo;
        TRACE_DEBUG("lct_decode phone, trade_id[%s] db_phone[%s]", row[1],row[7]);
        if((iRet = lct_decode(row[1], row[7], strDeTo))){
            gPtrAppLog->error("[%s][%d] lct_decode Fphone error! lct_decode Ftrade_id=%s db_Fphone=%s iRet=%d", 
                                __FILE__,__LINE__, row[1],row[7],iRet);
        }        
        strncpy(pstRecord->Fphone, ValiStr((char*)strDeTo.c_str()), sizeof(pstRecord->Fphone)-1);
    }else{
        strncpy(pstRecord->Fphone, ValiStr(row[7]), sizeof(pstRecord->Fphone)-1);
    }

    //长度大于DB_DECODE_CRE_ID_MAX_LENGTH表示加密后的数据才需解密，否则直接返回DB明文数据
    //由于mobile字段外部使用场景很少，故解码错误的异常暂时只记录错误，不抛异常
    if( DB_DECODE_MOBLIE_MAX_LENGTH < strlen(row[8]) ){
        string strDeTo;
        TRACE_DEBUG("lct_decode moblie, trade_id[%s] db_moblie[%s]", row[1],row[8]);
        if((iRet = lct_decode(row[1], row[8], strDeTo))){
            gPtrAppLog->error("[%s][%d] lct_decode Fmobile error! lct_decode Ftrade_id=%s db_Fmobile=%s iRet=%d", 
                                __FILE__,__LINE__, row[1],row[8],iRet);
        }        
        strncpy(pstRecord->Fmobile, ValiStr((char*)strDeTo.c_str()), sizeof(pstRecord->Fmobile)-1);
    }else{
        strncpy(pstRecord->Fmobile, ValiStr(row[8]), sizeof(pstRecord->Fmobile)-1);
    }
    strncpy(pstRecord->Fqqid, ValiStr(row[3]), sizeof(pstRecord->Fqqid)-1);
    pstRecord->Fuid = atoi(ValiStr(row[4]));
    strncpy(pstRecord->Ftrue_name, ValiStr(row[5]), sizeof(pstRecord->Ftrue_name)-1);
    //strncpy(pstRecord->Fcre_id_orig, ValiStr(row[6]), sizeof(pstRecord->Fcre_id_orig)-1);
    //strncpy(pstRecord->Fphone, ValiStr(row[7]), sizeof(pstRecord->Fphone)-1);
    //strncpy(pstRecord->Fmobile, ValiStr(row[8]), sizeof(pstRecord->Fmobile)-1);
    pstRecord->Fstate = atoi(ValiStr(row[9]));
    pstRecord->Flstate = atoi(ValiStr(row[10]));
    strncpy(pstRecord->Fopenid, ValiStr(row[11]), sizeof(pstRecord->Fopenid)-1);
    pstRecord->Facct_type= atoi(ValiStr(row[12]));
    strncpy(pstRecord->Fchannel_id, ValiStr(row[13]), sizeof(pstRecord->Fchannel_id)-1);
    pstRecord->Fasset_limit_lev= atoi(ValiStr(row[14]));
    strncpy(pstRecord->Fcreate_time, ValiStr(row[15]), sizeof(pstRecord->Fcreate_time)-1);
    strncpy(pstRecord->Fassess_modify_time, ValiDateStr(row[16]), sizeof(pstRecord->Fassess_modify_time)-1);
    pstRecord->Fassess_risk_type= atoi(ValiStr(row[17]));
    strncpy(pstRecord->Femail, ValiStr(row[18]), sizeof(pstRecord->Femail)-1);
    strncpy(pstRecord->Faddress, ValiStr(row[19]), sizeof(pstRecord->Faddress)-1);
	pstRecord->Ffrozen_channle = atoi(ValiStr(row[20]));
	strncpy(pstRecord->Fsign, ValiStr(row[21]), sizeof(pstRecord->Fsign) - 1);
    checkSign( "t_fund_bind", *pstRecord);
    mysql_free_result(result);

    TRACE_DEBUG("query fund bind exist");
    return true;
}

/**
 * 用证件号码查询基金用户绑定关系
 * 返回: 查询到记录返回true, 没有查询到记录返回false
 */
bool QueryFundBindByCre(CMySQL* mysql, int cre_type, const char* cre_id, 
                        ST_FUND_BIND* pstRecord, bool islock /* = false */)
{
    stringstream ss_cond;
    ss_cond << "Fcre_type=" << cre_type;
    ss_cond << " and Fcre_id='" << escapeString(cre_id) << "'";
    if (islock)
    {
        ss_cond << " for update";
    }

    return QueryFundBind(mysql, ss_cond.str(), pstRecord);
}


/**
 * 用uid查询基金用户绑定关系表有效记录
 * 返回: 查询到记录返回true, 没有查询到记录返回false
 */
bool QueryFundBindByUid(CMySQL* mysql, int uid, ST_FUND_BIND* pstRecord, bool islock /* = false */)
{
    stringstream ss_cond;
	ss_cond << "Fuid=" << uid << " and Flstate in (1,3)";
    if (islock)
    {
        ss_cond << " for update";
    }

    return QueryFundBind(mysql, ss_cond.str(), pstRecord);
}

/**
 * 用uin查询基金用户绑定关系表有效记录
 * 返回: 查询到记录返回true, 没有查询到记录返回false
 */
bool QueryFundBindByUin(CMySQL* mysql, const string &uin, ST_FUND_BIND* pstRecord, bool islock /* = false */)
{
    stringstream ss_cond;
	ss_cond << "Fqqid='" << escapeString(uin) << "' and Flstate in (1,3)";
    if (islock)
    {
        ss_cond << " for update";
    }

    return QueryFundBind(mysql, ss_cond.str(), pstRecord);
}



/**
 * 用trade_id查询基金用户绑定关系
 * 返回: 查询到记录返回true, 没有查询到记录返回false
 */

bool QueryFundBindByTradeid(CMySQL* mysql, const char* trade_id, ST_FUND_BIND* pstRecord, bool islock,bool valid)
{
    stringstream ss_cond;
    ss_cond << "Ftrade_id='" << escapeString(trade_id) << "'";
    if (valid)
    {
        ss_cond << " and Flstate in (1,3) ";
    }
    if (islock)
    {
        ss_cond << " for update";
    }



    return QueryFundBind(mysql, ss_cond.str(), pstRecord);
}


/**
 * 用单号查询基金交易记录是否存在
 * 返回: 查询到记录返回true, 没有查询到记录返回false
 */
bool QueryTradeFund(CMySQL* mysql, const char* listid, int pur_type, ST_TRADE_FUND* pstRecord, bool islock)
{
    char    szSqlStr[MAX_SQL_LEN*2+1];
    MYSQL_RES*    pResult = NULL;
    MYSQL_ROW    row;
    char    szSubStr[256];

    memset(szSqlStr, 0, sizeof(szSqlStr));
    memset(szSubStr, 0, sizeof(szSubStr));

    // 查询基金购买记录，需要考虑三种交易类型
    if(PURTYPE_BUY == pur_type)
    {
        snprintf(szSubStr, sizeof(szSubStr), "(Fpur_type=1 or Fpur_type=2 or Fpur_type=3 or Fpur_type=9 or Fpur_type=10 or Fpur_type=11 or Fpur_type=12)");
    }
    else
    {
        snprintf(szSubStr, sizeof(szSubStr), "Fpur_type=%d", pur_type);
    }

    string strLock = islock ? "FOR UPDATE" : "";

    // 转义单号
    string sEscapelistId = escapeString(listid);
    
    int iLen = snprintf(szSqlStr, sizeof(szSqlStr), 
        "select Flistid, Fspid, Fcoding, Ftrade_id, Fuid, Ffund_name, Ffund_code, Fpur_type, Ftotal_fee, "
        "Fbank_type, Fcard_no, Fstate, Flstate, Ftrade_date, Ffund_value, Ffund_vdate, Ffund_type, Fnotify_url, "
        "Frela_listid, Fdrawid, Ffetchid, Fcft_timestamp, Fcreate_time, Fstandby1,Fcft_bank_billno,Fsub_trans_id, "
        "Fcur_type, Fspe_tag, Facc_time, Fpurpose,Fcft_trans_id,Floading_type,Fchannel_id,Fclose_listid,Fopt_type, "
        "Freal_redem_amt,Fend_date,Fstandby4,Fcft_charge_ctrl_id,Fcharge_type,Fcharge_fee,Fsign,Fpay_channel "
        "from fund_db_%02d.t_trade_fund_%d "
        "where Flistid='%s' and %s %s", 
        Sdb2(sEscapelistId.c_str()), Stb2(sEscapelistId.c_str()), sEscapelistId.c_str(), szSubStr, strLock.c_str());

	gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSqlStr );

    mysql->Query(szSqlStr, iLen);

    pResult = mysql->FetchResult();
    if(0 == mysql_num_rows(pResult))
    {
        mysql_free_result(pResult);
        return false;
    }

    row = mysql_fetch_row(pResult);

    strncpy(pstRecord->Flistid, ValiStr(row[0]), sizeof(pstRecord->Flistid)-1);
    strncpy(pstRecord->Fspid, ValiStr(row[1]), sizeof(pstRecord->Fspid)-1);
    strncpy(pstRecord->Fcoding, ValiStr(row[2]), sizeof(pstRecord->Fcoding)-1);
    strncpy(pstRecord->Ftrade_id, ValiStr(row[3]), sizeof(pstRecord->Ftrade_id)-1);
    pstRecord->Fuid = atoi(ValiStr(row[4]));
    strncpy(pstRecord->Ffund_name, ValiStr(row[5]), sizeof(pstRecord->Ffund_name)-1);
    strncpy(pstRecord->Ffund_code, ValiStr(row[6]), sizeof(pstRecord->Ffund_code)-1);
    pstRecord->Fpur_type = atoi(ValiStr(row[7]));
    pstRecord->Ftotal_fee = atoll(ValiStr(row[8]));
    pstRecord->Fbank_type = atoi(ValiStr(row[9]));
    strncpy(pstRecord->Fcard_no, ValiStr(row[10]), sizeof(pstRecord->Fcard_no)-1);
    pstRecord->Fstate = atoi(ValiStr(row[11]));
    pstRecord->Flstate = atoi(ValiStr(row[12]));
    strncpy(pstRecord->Ftrade_date, ValiStr(row[13]), sizeof(pstRecord->Ftrade_date)-1);
    strncpy(pstRecord->Ffund_value, ValiStr(row[14]), sizeof(pstRecord->Ffund_value)-1);
    strncpy(pstRecord->Ffund_vdate, ValiStr(row[15]), sizeof(pstRecord->Ffund_vdate)-1);
    strncpy(pstRecord->Ffund_type, ValiStr(row[16]), sizeof(pstRecord->Ffund_type)-1);
    strncpy(pstRecord->Fnotify_url, ValiStr(row[17]), sizeof(pstRecord->Fnotify_url)-1);
    strncpy(pstRecord->Frela_listid, ValiStr(row[18]), sizeof(pstRecord->Frela_listid)-1);
    strncpy(pstRecord->Fdrawid, ValiStr(row[19]), sizeof(pstRecord->Fdrawid)-1);
    strncpy(pstRecord->Ffetchid, ValiStr(row[20]), sizeof(pstRecord->Ffetchid)-1);
    pstRecord->Fcft_timestamp = atoi(ValiStr(row[21]));
	strncpy(pstRecord->Fcreate_time, ValiStr(row[22]), sizeof(pstRecord->Fcreate_time)-1);
    pstRecord->Fstandby1 = atoi(ValiStr(row[23]));
	strncpy(pstRecord->Fcft_bank_billno, ValiStr(row[24]), sizeof(pstRecord->Fcft_bank_billno)-1);
	strncpy(pstRecord->Fsub_trans_id, ValiStr(row[25]), sizeof(pstRecord->Fsub_trans_id)-1);
	pstRecord->Fcur_type = atoi(ValiStr(row[26]));
	pstRecord->Fspe_tag = atoi(ValiStr(row[27]));
	strncpy(pstRecord->Facc_time, ValiStr(row[28]), sizeof(pstRecord->Facc_time)-1);
	pstRecord->Fpurpose = atoi(ValiStr(row[29]));
	strncpy(pstRecord->Fcft_trans_id, ValiStr(row[30]), sizeof(pstRecord->Fcft_trans_id)-1);
    pstRecord->Floading_type = atoi(ValiStr(row[31]));
    strncpy(pstRecord->Fchannel_id, ValiStr(row[32]), sizeof(pstRecord->Fchannel_id)-1);
	pstRecord->Fclose_listid= atoll(ValiStr(row[33]));
	pstRecord->Fopt_type= atoi(ValiStr(row[34]));
	pstRecord->Freal_redem_amt= atoll(ValiStr(row[35]));
	strncpy(pstRecord->Fend_date, ValiStr(row[36]), sizeof(pstRecord->Fend_date)-1);
	strncpy(pstRecord->Fcoupon_id, ValiStr(row[37]), sizeof(pstRecord->Fcoupon_id)-1);
	strncpy(pstRecord->Fcft_charge_ctrl_id, ValiStr(row[38]), sizeof(pstRecord->Fcft_charge_ctrl_id)-1);
	strncpy(pstRecord->Fcharge_type, ValiStr(row[39]), sizeof(pstRecord->Fcharge_type)-1);
	pstRecord->Fcharge_fee= atoll(ValiStr(row[40]));
    strncpy(pstRecord->Fsign, ValiStr(row[41]), sizeof(pstRecord->Fsign)-1);
    checkSign( "t_trade_fund", *pstRecord);
	pstRecord->Fpay_channel = atoi(ValiStr(row[42]));
    
    mysql_free_result(pResult);
    return true;
}

/**
 * 批量查询用户交易记录
 * 返回: 查询到记录返回true, 没有查询到记录返回false
 */
bool QueryBatchTradeFund(CMySQL* mysql, ST_TRADE_FUND& input, vector<ST_TRADE_FUND>& dataVec, bool islock)
{
    char    szSqlStr[MAX_SQL_LEN*2+1];
    MYSQL_RES*    pResult = NULL;
    char    szSubStr[256];

    memset(szSqlStr, 0, sizeof(szSqlStr));
    memset(szSubStr, 0, sizeof(szSubStr));

    
    string strLock = islock ? "FOR UPDATE" : "";
    
    int iLen = snprintf(szSqlStr, sizeof(szSqlStr), 
        "select Flistid, Fspid, Fcoding, Ftrade_id, Fuid, Ffund_name, Ffund_code, Fpur_type, Ftotal_fee, "
        "Fbank_type, Fcard_no, Fstate, Flstate, Ftrade_date, Ffund_value, Ffund_vdate, Ffund_type, Fnotify_url, "
        "Frela_listid, Fdrawid, Ffetchid, Fcft_timestamp, Fcreate_time, Fstandby1,Fcft_bank_billno,Fsub_trans_id, "
        "Fcur_type, Fspe_tag, Facc_time, Fpurpose,Fcft_trans_id,Floading_type,Fchannel_id,Fclose_listid,Fopt_type, "
        "Freal_redem_amt,Fend_date,Frefund_type,Ffetch_arrival_time,Ffetch_result,Fcharge_type,Fcharge_fee,Fsign,  "
        "Fpay_channel "
        "from fund_db_%02d.t_trade_user_fund_%d "
        "where Fuid='%d' and Flstate=1 "
        " and Fpur_type in (1,4,10,11,12) "
        " and Fstate in (2,3,4,5,6,8,9,10,12,13) "
        " order by Facc_time desc limit 21 ", 
        Sdb1(input.Fuid), Stb1(input.Fuid), input.Fuid);

    mysql->Query(szSqlStr, iLen);

    pResult = mysql->FetchResult();
	int iRow = mysql_num_rows(pResult);
    if(iRow <0 )
    {
        throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
    }
    if(0 == mysql_num_rows(pResult))
    {
        mysql_free_result(pResult);
        return false;
    }

	for(int i=0; i<iRow; i++) 
	{
		MYSQL_ROW row = mysql_fetch_row(pResult);
		ST_TRADE_FUND data;
		memset(&data, 0,sizeof(ST_TRADE_FUND));
		
		strncpy(data.Flistid, ValiStr(row[0]), sizeof(data.Flistid)-1);
		strncpy(data.Fspid, ValiStr(row[1]), sizeof(data.Fspid)-1);
		strncpy(data.Fcoding, ValiStr(row[2]), sizeof(data.Fcoding)-1);
		strncpy(data.Ftrade_id, ValiStr(row[3]), sizeof(data.Ftrade_id)-1);
		data.Fuid = atoi(ValiStr(row[4]));
		strncpy(data.Ffund_name, ValiStr(row[5]), sizeof(data.Ffund_name)-1);
		strncpy(data.Ffund_code, ValiStr(row[6]), sizeof(data.Ffund_code)-1);
		data.Fpur_type = atoi(ValiStr(row[7]));
		data.Ftotal_fee = atoll(ValiStr(row[8]));
		data.Fbank_type = atoi(ValiStr(row[9]));
		strncpy(data.Fcard_no, ValiStr(row[10]), sizeof(data.Fcard_no)-1);
		data.Fstate = atoi(ValiStr(row[11]));
		data.Flstate = atoi(ValiStr(row[12]));
		strncpy(data.Ftrade_date, ValiStr(row[13]), sizeof(data.Ftrade_date)-1);
		strncpy(data.Ffund_value, ValiStr(row[14]), sizeof(data.Ffund_value)-1);
		strncpy(data.Ffund_vdate, ValiStr(row[15]), sizeof(data.Ffund_vdate)-1);
		strncpy(data.Ffund_type, ValiStr(row[16]), sizeof(data.Ffund_type)-1);
		strncpy(data.Fnotify_url, ValiStr(row[17]), sizeof(data.Fnotify_url)-1);
		strncpy(data.Frela_listid, ValiStr(row[18]), sizeof(data.Frela_listid)-1);
		strncpy(data.Fdrawid, ValiStr(row[19]), sizeof(data.Fdrawid)-1);
		strncpy(data.Ffetchid, ValiStr(row[20]), sizeof(data.Ffetchid)-1);
		data.Fcft_timestamp = atoi(ValiStr(row[21]));
		strncpy(data.Fcreate_time, ValiStr(row[22]), sizeof(data.Fcreate_time)-1);
		data.Fstandby1 = atoi(ValiStr(row[23]));
		strncpy(data.Fcft_bank_billno, ValiStr(row[24]), sizeof(data.Fcft_bank_billno)-1);
		strncpy(data.Fsub_trans_id, ValiStr(row[25]), sizeof(data.Fsub_trans_id)-1);
		data.Fcur_type = atoi(ValiStr(row[26]));
		data.Fspe_tag = atoi(ValiStr(row[27]));
		strncpy(data.Facc_time, ValiStr(row[28]), sizeof(data.Facc_time)-1);
		data.Fpurpose = atoi(ValiStr(row[29]));
		strncpy(data.Fcft_trans_id, ValiStr(row[30]), sizeof(data.Fcft_trans_id)-1);
    	data.Floading_type = atoi(ValiStr(row[31]));
        strncpy(data.Fchannel_id, ValiStr(row[32]), sizeof(data.Fchannel_id)-1);
		data.Fclose_listid= atoll(ValiStr(row[33]));
		data.Fopt_type= atoi(ValiStr(row[34]));
		data.Freal_redem_amt= atoll(ValiStr(row[35]));
		strncpy(data.Fend_date, ValiStr(row[36]), sizeof(data.Fend_date)-1);
		data.Frefund_type= atoi(ValiStr(row[37]));
		strncpy(data.Ffetch_arrival_time, ValiStr(row[38]), sizeof(data.Ffetch_arrival_time)-1);
		data.Ffetch_result = atoi(ValiStr(row[39]));
		strncpy(data.Fcharge_type, ValiStr(row[40]), sizeof(data.Fcharge_type)-1);
		data.Fcharge_fee= atoll(ValiStr(row[41]));
        strncpy(data.Fsign, ValiStr(row[42]), sizeof(data.Fsign)-1);
        checkSign( "t_trade_user_fund", data);
        data.Fpay_channel = atoi(ValiStr(row[43]));
    	dataVec.push_back(data);

    }
    mysql_free_result(pResult);
	
    return iRow >= 1;
}


/**
 * 用财付通给银行订单号查询基金交易记录是否存在
 * 返回: 查询到记录返回true, 没有查询到记录返回false
 */
bool QueryTradeFundByBankBillno(CMySQL* mysql, const char* cft_bank_billno, int bank_type, ST_TRADE_FUND* pstRecord, bool islock)
{
    char    szSqlStr[MAX_SQL_LEN*2+1];
    MYSQL_RES*    pResult = NULL;
    MYSQL_ROW    row;


    memset(szSqlStr, 0, sizeof(szSqlStr));

    string strLock = islock ? "FOR UPDATE" : "";

    // 转义单号
    string sEscapeBankBillno = escapeString(cft_bank_billno);
    
    int iLen = snprintf(szSqlStr, sizeof(szSqlStr), 
        "select Flistid, Fspid, Fcoding, Ftrade_id, Fuid, Ffund_name, Ffund_code, Fpur_type, Ftotal_fee, "
        "Fbank_type, Fcard_no, Fstate, Flstate, Ftrade_date, Ffund_value, Ffund_vdate, Ffund_type, Fnotify_url, "
        "Frela_listid, Fdrawid, Ffetchid, Fcft_timestamp, Fcreate_time, Fstandby1,Fcft_bank_billno,Fsub_trans_id, "
        "Fcur_type, Fspe_tag, Facc_time, Fpurpose,Fcft_trans_id,Floading_type,Fchannel_id,Fclose_listid,Fopt_type, "
        "Freal_redem_amt,Fend_date,Fcharge_type,Fcharge_fee,Fsign,Fpay_channel "
        "from fund_db_%02d.t_trade_fund_%d "
        "where Fcft_bank_billno='%s' and Fbank_type=%d %s", 
        Sdb2(sEscapeBankBillno.c_str()), Stb2(sEscapeBankBillno.c_str()), sEscapeBankBillno.c_str(), bank_type, strLock.c_str());

    mysql->Query(szSqlStr, iLen);

    pResult = mysql->FetchResult();
    if(0 == mysql_num_rows(pResult))
    {
        mysql_free_result(pResult);
        return false;
    }

    row = mysql_fetch_row(pResult);

    strncpy(pstRecord->Flistid, ValiStr(row[0]), sizeof(pstRecord->Flistid)-1);
    strncpy(pstRecord->Fspid, ValiStr(row[1]), sizeof(pstRecord->Fspid)-1);
    strncpy(pstRecord->Fcoding, ValiStr(row[2]), sizeof(pstRecord->Fcoding)-1);
    strncpy(pstRecord->Ftrade_id, ValiStr(row[3]), sizeof(pstRecord->Ftrade_id)-1);
    pstRecord->Fuid = atoi(ValiStr(row[4]));
    strncpy(pstRecord->Ffund_name, ValiStr(row[5]), sizeof(pstRecord->Ffund_name)-1);
    strncpy(pstRecord->Ffund_code, ValiStr(row[6]), sizeof(pstRecord->Ffund_code)-1);
    pstRecord->Fpur_type = atoi(ValiStr(row[7]));
    pstRecord->Ftotal_fee = atoll(ValiStr(row[8]));
    pstRecord->Fbank_type = atoi(ValiStr(row[9]));
    strncpy(pstRecord->Fcard_no, ValiStr(row[10]), sizeof(pstRecord->Fcard_no)-1);
    pstRecord->Fstate = atoi(ValiStr(row[11]));
    pstRecord->Flstate = atoi(ValiStr(row[12]));
    strncpy(pstRecord->Ftrade_date, ValiStr(row[13]), sizeof(pstRecord->Ftrade_date)-1);
    strncpy(pstRecord->Ffund_value, ValiStr(row[14]), sizeof(pstRecord->Ffund_value)-1);
    strncpy(pstRecord->Ffund_vdate, ValiStr(row[15]), sizeof(pstRecord->Ffund_vdate)-1);
    strncpy(pstRecord->Ffund_type, ValiStr(row[16]), sizeof(pstRecord->Ffund_type)-1);
    strncpy(pstRecord->Fnotify_url, ValiStr(row[17]), sizeof(pstRecord->Fnotify_url)-1);
    strncpy(pstRecord->Frela_listid, ValiStr(row[18]), sizeof(pstRecord->Frela_listid)-1);
    strncpy(pstRecord->Fdrawid, ValiStr(row[19]), sizeof(pstRecord->Fdrawid)-1);
    strncpy(pstRecord->Ffetchid, ValiStr(row[20]), sizeof(pstRecord->Ffetchid)-1);
    pstRecord->Fcft_timestamp = atoi(ValiStr(row[21]));
	strncpy(pstRecord->Fcreate_time, ValiStr(row[22]), sizeof(pstRecord->Fcreate_time)-1);
    pstRecord->Fstandby1 = atoi(ValiStr(row[23]));
	strncpy(pstRecord->Fcft_bank_billno, ValiStr(row[24]), sizeof(pstRecord->Fcft_bank_billno)-1);
	strncpy(pstRecord->Fsub_trans_id, ValiStr(row[25]), sizeof(pstRecord->Fsub_trans_id)-1);
	pstRecord->Fcur_type = atoi(ValiStr(row[26]));
	pstRecord->Fspe_tag = atoi(ValiStr(row[27]));
	strncpy(pstRecord->Facc_time, ValiStr(row[28]), sizeof(pstRecord->Facc_time)-1);
	pstRecord->Fpurpose = atoi(ValiStr(row[29]));
	strncpy(pstRecord->Fcft_trans_id, ValiStr(row[30]), sizeof(pstRecord->Fcft_trans_id)-1);
    pstRecord->Floading_type = atoi(ValiStr(row[31]));
	strncpy(pstRecord->Fchannel_id, ValiStr(row[32]), sizeof(pstRecord->Fchannel_id)-1);
	pstRecord->Fclose_listid= atoll(ValiStr(row[33]));
	pstRecord->Fopt_type= atoi(ValiStr(row[34]));
	pstRecord->Freal_redem_amt= atoll(ValiStr(row[35]));
	strncpy(pstRecord->Fend_date, ValiStr(row[36]), sizeof(pstRecord->Fend_date)-1);
	strncpy(pstRecord->Fcharge_type, ValiStr(row[37]), sizeof(pstRecord->Fcharge_type)-1);
	pstRecord->Fcharge_fee= atoll(ValiStr(row[38]));
	strncpy(pstRecord->Fsign, ValiStr(row[39]), sizeof(pstRecord->Fsign)-1);
    checkSign( "t_trade_fund", *pstRecord);
    pstRecord->Fpay_channel = atoi(ValiStr(row[40]));
	 	
    mysql_free_result(pResult);
    return true;
}


/**
 * 按用户统计某日的T+1赎回总份额(T-2之后到recondDay之前的总份额)
 * 返回: 查询结果
 */
LONG StatTplusRedemFee(CMySQL* mysql, const string& recondDay,const string& trade_id,int curtype)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    string start_time;
    LONG fee=0;

    if (trade_id.length() <3)
    {
        throw CException(ERR_DB_UNKNOW, "in StatTplusRedemFee invalid input trade_id!", __FILE__, __LINE__);
    }
    string Tminus2Date;
    string TminusDate;
    bool isCurTDay = false;
    getTminus2TransDate(mysql,recondDay,Tminus2Date,TminusDate,isCurTDay);
    if (TminusDate.empty() ||Tminus2Date.empty() )
    {
        throw CException(ERR_DB_UNKNOW, "get T-2 transday fail from db!", __FILE__, __LINE__);
    }

    if (isCurTDay == true) //今天是交易日
    {
        start_time = TminusDate;
    }
    else
    {
        start_time = Tminus2Date;
    }

    if (start_time == addDays(recondDay, -1))
    {
        return 0;
    }
    
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " sum(Ftplus_redem_money) "
                    " FROM fund_db_%s.t_fund_profit_record_%s "
                    " WHERE "
                    " Ftrade_id='%s'  AND " 
                    " Fday > '%s' AND "
                    " Fday < '%s' AND "
                    " Fcurtype=%d  "
                    ,
                    trade_id.substr(trade_id.length()-2,2).c_str(),
                    trade_id.substr(trade_id.length()-3,1).c_str(),
                    mysql->EscapeStr(trade_id).c_str(),
                    mysql->EscapeStr(start_time).c_str(),
                    mysql->EscapeStr(recondDay).c_str(),
                    curtype
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        mysql->Query(szSql, iLen);
        // 取结果集
        pRes = mysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        
        MYSQL_ROW row = mysql_fetch_row(pRes);  
        fee = row[0] ? atoll(row[0]) : 0;

        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return fee;
}

/**
 * 按用户统计未确认的历史申购金额或者赎回份额
 * 返回: 查询到记录返回true, 没有查询到记录返回false
 */
bool StatHisotryNotAckFee(CMySQL* mysql,ST_TRADE_FUND& pstRecord,LONG &fee,string purTypeConf, bool islock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    string start_time;
   
    string recondDay = string(pstRecord.Ftrade_date);
    string Tminus2Date;
    string TminusDate;
    bool isCurTDay = false;
    getTminus2TransDate(mysql,recondDay,Tminus2Date,TminusDate,isCurTDay);

	//申购未确认总金额 = (<=入账日期  第二个T日)15点到入账日期的15点
	//赎回未确认的冻结份额= 	 (<=入账日期  第二个T日)15点到入账日期的15点

	if (TminusDate.empty()  ||  Tminus2Date.empty())
	{
		throw CException(ERR_DB_UNKNOW, "get T-2 transday fail from db!", __FILE__, __LINE__);
	}

	if (isCurTDay == false) //今天不是 交易日
	{
		TminusDate = Tminus2Date;
	}	
	if(gPtrConfig->m_AppCfg.trans_recon_type == 1)
	{
		start_time = changeDateFormat(TminusDate) + " 15:00:00";
		recondDay = changeDateFormat(recondDay) + " 14:59:59";	
	}
	else
	{
	     start_time = changeDateFormat(TminusDate) + " 00:00:00";
         recondDay = changeDateFormat(recondDay) +  " 23:59:59";
	}
	
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " sum(Ftotal_fee) "
                    " FROM fund_db_%02d.t_trade_user_fund_%d "
                    " WHERE "
                    " Ftrade_id='%s'  AND " 
                    " Facc_time >= '%s' AND "
                    " Facc_time <= '%s' AND "
                    " Fcur_type=%d AND "
                    " (Fstate =2 or Fstate=12 or Fstate =13 ) AND Flstate=1 AND "
                    " %s "
                    " %s ",
                    Sdb1(pstRecord.Fuid),
                    Stb1(pstRecord.Fuid),
                    mysql->EscapeStr(pstRecord.Ftrade_id).c_str(),
                    mysql->EscapeStr(start_time).c_str(),
                    mysql->EscapeStr(recondDay).c_str(),
                    pstRecord.Fcur_type,
                    purTypeConf.c_str(),
                    islock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        mysql->Query(szSql, iLen);
        // 取结果集
        pRes = mysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        
        MYSQL_ROW row = mysql_fetch_row(pRes);  
		fee = row[0] ? atoll(row[0]) : 0;

        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return iRow == 1;
}

/**
 * 按用户统计当时确认的申购份额
 * 返回: 查询到记录返回true, 没有查询到记录返回false
 */
bool StatTodayAckPurchaseFee(CMySQL* mysql,ST_TRADE_FUND& pstRecord,LONG &fee,string purTypeConf, bool islock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    string start_time;
    string  end_time;
    string recondDay = string(pstRecord.Ftrade_date);
    string Tminus2Date;
    string TminusDate;
    bool isCurTDay = false;
    getTminus2TransDate(mysql,recondDay,Tminus2Date,TminusDate,isCurTDay);

	//非交易日为0，交易日用下面的计算
	//----------	开始时间 <=入账时间的 获取三个T日:入账日期为交易日取第三个
	//-----------结束时间 <=入账时间的 获取三个T日:入账日期为交易日取第二个
	if (TminusDate.empty()  ||  Tminus2Date.empty())
    {
        throw CException(ERR_DB_UNKNOW, "get T-2 transday fail from db!", __FILE__, __LINE__);
    }
	
	if (isCurTDay == true) //今天是交易日
	{
		   start_time = Tminus2Date;
		   end_time = TminusDate;
	}
	else
	{
		  fee = 0;
		  return true;
    }

	if(gPtrConfig->m_AppCfg.trans_recon_type == 1)
	{
		start_time = changeDateFormat(start_time) + " 15:00:00";
		end_time = changeDateFormat(end_time) + " 14:59:59";
	}
	else
	{
	     start_time = changeDateFormat(start_time) + " 00:00:00";
         end_time = changeDateFormat(end_time) +  " 23:59:59";
	}
	
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " sum(Freal_redem_amt) "
                    " FROM fund_db_%02d.t_trade_user_fund_%d "
                    " WHERE "
                    " Ftrade_id='%s'  AND " 
                    " Facc_time >= '%s' AND "
                    " Facc_time <= '%s' AND "
                    " Fcur_type=%d AND "
                    " (Fstate =3  ) AND Flstate=1 AND "
                    " %s "
                    " %s ",
                    Sdb1(pstRecord.Fuid),
                    Stb1(pstRecord.Fuid),
                    mysql->EscapeStr(pstRecord.Ftrade_id).c_str(),
                    mysql->EscapeStr(start_time).c_str(),
                    mysql->EscapeStr(end_time).c_str(),
                    pstRecord.Fcur_type,
                    purTypeConf.c_str(),
                    islock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        mysql->Query(szSql, iLen);
        // 取结果集
        pRes = mysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        
        MYSQL_ROW row = mysql_fetch_row(pRes);  
		fee = row[0] ? atoll(row[0]) : 0;

        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return iRow == 1;
}
/**
 * 按用户统计当时确认的赎回的份额
 * 返回: 查询到记录返回true, 没有查询到记录返回false
 */
bool StatTodayAckRedemFee(CMySQL* mysql,ST_TRADE_FUND& pstRecord,LONG &fee,string purTypeConf, bool islock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    string start_time;
    string  end_time;
    string recondDay = string(pstRecord.Ftrade_date);
    string Tminus2Date;
    string TminusDate;
    bool isCurTDay = false;
    getTminus2TransDate(mysql,recondDay,Tminus2Date,TminusDate,isCurTDay);

	//非交易日为0，交易日用下面的计算
	//----------	开始时间 <=入账时间的 获取三个T日:入账日期为交易日取第三个
	//-----------结束时间 <=入账时间的 获取三个T日:入账日期为交易日取第二个
	if (TminusDate.empty()  ||  Tminus2Date.empty())
    {
        throw CException(ERR_DB_UNKNOW, "get T-2 transday fail from db!", __FILE__, __LINE__);
    }
	
	if (isCurTDay == true) //今天是交易日
	{
		   start_time = Tminus2Date;
		   end_time = TminusDate;
	}
	else
	{
		  fee = 0;
		  return true;
    }

	if(gPtrConfig->m_AppCfg.trans_recon_type == 1)
	{
		start_time = changeDateFormat(start_time) + " 15:00:00";
		end_time = changeDateFormat(end_time) + " 14:59:59";
	}
	else
	{
	     start_time = changeDateFormat(start_time) + " 00:00:00";
         end_time = changeDateFormat(end_time) +  " 23:59:59";
	}
	
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " sum(Ftotal_fee) "
                    " FROM fund_db_%02d.t_trade_user_fund_%d "
                    " WHERE "
                    " Ftrade_id='%s'  AND " 
                    " Facc_time >= '%s' AND "
                    " Facc_time <= '%s' AND "
                    " Fcur_type=%d AND "
                    " (Fstate =5 or Fstate =10 ) AND Flstate=1 AND "
                    " %s "
                    " %s ",
                    Sdb1(pstRecord.Fuid),
                    Stb1(pstRecord.Fuid),
                    mysql->EscapeStr(pstRecord.Ftrade_id).c_str(),
                    mysql->EscapeStr(start_time).c_str(),
                    mysql->EscapeStr(end_time).c_str(),
                    pstRecord.Fcur_type,
                    purTypeConf.c_str(),
                    islock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        mysql->Query(szSql, iLen);
        // 取结果集
        pRes = mysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        
        MYSQL_ROW row = mysql_fetch_row(pRes);  
		fee = row[0] ? atoll(row[0]) : 0;

        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return iRow == 1;
}

/**
 * 按用户统计某日的某交易类型的交易总额
 * 返回: 查询到记录返回true, 没有查询到记录返回false
 */
bool StatTransFee(CMySQL* mysql, ST_TRADE_FUND& pstRecord, LONG& fee, string purTypeConf, bool islock)
{
	MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
	string start_time,end_time;
	
	if(gPtrConfig->m_AppCfg.trans_recon_type == 1)
	{
		start_time = changeDateFormat(addDays(pstRecord.Ftrade_date,-1)) + " 15:00:00";
		end_time = changeDateFormat(pstRecord.Ftrade_date) + " 14:59:59";	
	}
	else
	{
		start_time = pstRecord.Ftrade_date;
		end_time = changeDateFormat(pstRecord.Ftrade_date) + " 23:59:59";
	}
	
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " sum(Ftotal_fee) "
                    " FROM fund_db_%02d.t_trade_user_fund_%d "
                    " WHERE "
                    " Ftrade_id='%s'  AND " 
                    " Facc_time >= '%s' AND "
                    " Facc_time <= '%s' AND "
                    " Fcur_type=%d AND "
                    " (Fstate=2 or Fstate=3 or Fstate=5 or Fstate=10) AND Flstate=1 AND "
                    " %s "
                    " %s ",
                    Sdb1(pstRecord.Fuid),
                    Stb1(pstRecord.Fuid),
                    mysql->EscapeStr(pstRecord.Ftrade_id).c_str(),
                    mysql->EscapeStr(start_time).c_str(),
                    mysql->EscapeStr(end_time).c_str(),
                    pstRecord.Fcur_type,
                    purTypeConf.c_str(),
                    islock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        mysql->Query(szSql, iLen);
        // 取结果集
        pRes = mysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        
        MYSQL_ROW row = mysql_fetch_row(pRes);  
		fee = row[0] ? atoll(row[0]) : 0;

        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return iRow == 1;
}


/**
 * 按用户统计某日的某交易类型的未确认申购金额或者赎回份额
 * 返回: 查询到记录返回true, 没有查询到记录返回false
 */
bool StatTransNotAckFee(CMySQL* mysql, ST_TRADE_FUND& pstRecord, LONG& fee, string purTypeConf, bool islock)
{
	MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
	string start_time,end_time;
	
	if(gPtrConfig->m_AppCfg.trans_recon_type == 1)
	{
		start_time = changeDateFormat(addDays(pstRecord.Ftrade_date,-1)) + " 15:00:00";
		end_time = changeDateFormat(pstRecord.Ftrade_date) + " 14:59:59";	
	}
	else
	{
		start_time = pstRecord.Ftrade_date;
		end_time = changeDateFormat(pstRecord.Ftrade_date) + " 23:59:59";
	}
	
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " sum(Ftotal_fee) "
                    " FROM fund_db_%02d.t_trade_user_fund_%d "
                    " WHERE "
                    " Ftrade_id='%s'  AND " 
                    " Facc_time >= '%s' AND "
                    " Facc_time <= '%s' AND "
                    " Fcur_type=%d AND "
                    " (Fstate =2 or Fstate=12 or Fstate =13 ) AND Flstate=1 AND "
                    " %s "
                    " %s ",
                    Sdb1(pstRecord.Fuid),
                    Stb1(pstRecord.Fuid),
                    mysql->EscapeStr(pstRecord.Ftrade_id).c_str(),
                    mysql->EscapeStr(start_time).c_str(),
                    mysql->EscapeStr(end_time).c_str(),
                    pstRecord.Fcur_type,
                    purTypeConf.c_str(),
                    islock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        mysql->Query(szSql, iLen);
        // 取结果集
        pRes = mysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        
        MYSQL_ROW row = mysql_fetch_row(pRes);  
		fee = row[0] ? atoll(row[0]) : 0;

        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return iRow == 1;
}

/**
 * 新增基金账户绑定关系表记录
 */
void InsertFundBind(CMySQL* mysql, ST_FUND_BIND* pstRecord)
{
	int iRet = 0;
    string strEncodeCreId;
    string strEncodeCreIdOrig;
    string strEncodeFphone;
    string strEncodeFmobile;
        
    //配置开关:是否写入加密后的敏感数据
    if( 1 == gPtrConfig->m_AppCfg.db_encode_switch )    
    {
        if((iRet = lct_encode(pstRecord->Ftrade_id, pstRecord->Fcre_id, strEncodeCreId))){
            strEncodeCreId.assign(pstRecord->Fcre_id);
            gPtrAppLog->error("[%s][%d] lct_encode Fcre_id error! lct_encode iRet=%d", __FILE__,__LINE__, iRet);
        }else{
            gPtrAppLog->debug("encode cre_id data result: Ftrade_id[%s] Fcre_id_encode[%s]", pstRecord->Ftrade_id, strEncodeCreId.c_str());
        }        
        
        if((iRet = lct_encode(pstRecord->Ftrade_id, pstRecord->Fcre_id_orig, strEncodeCreIdOrig))){
            strEncodeCreIdOrig.assign(pstRecord->Fcre_id_orig);
            gPtrAppLog->error("[%s][%d] lct_encode Fcre_id_orig error! lct_encode iRet=%d", __FILE__,__LINE__, iRet);
        }else{
            gPtrAppLog->debug("encode cre_id_orig data result: Ftrade_id[%s] Fcre_id_orig_encode[%s]", pstRecord->Ftrade_id, strEncodeCreIdOrig.c_str());
        }

        if((iRet = lct_encode(pstRecord->Ftrade_id, pstRecord->Fphone, strEncodeFphone))){
            strEncodeFphone.assign(pstRecord->Fphone);
            gPtrAppLog->error("[%s][%d] lct_encode Fphone error! lct_encode iRet=%d", __FILE__,__LINE__, iRet);
        }else{
            gPtrAppLog->debug("encode phone data result: Ftrade_id[%s] Fphone_encode[%s]", pstRecord->Ftrade_id, strEncodeFphone.c_str());
        }        

        if((iRet = lct_encode(pstRecord->Ftrade_id, pstRecord->Fmobile, strEncodeFmobile))){
            strEncodeFmobile.assign(pstRecord->Fmobile);
            gPtrAppLog->error("[%s][%d] lct_encode Fmobile error! lct_encode iRet=%d", __FILE__,__LINE__, iRet);
        }else{
            gPtrAppLog->debug("encode mobile data result: Ftrade_id[%s] Fmobile_encode[%s]", pstRecord->Ftrade_id, strEncodeFmobile.c_str());            
        }
    }
	
    char    szSqlStr[MAX_SQL_LEN*2+1] = {0};

    int iLen = snprintf(szSqlStr, sizeof(szSqlStr),
        "insert into fund_db.t_fund_bind "
        "(Fcre_type, Fcre_id, Ftrade_id, Fqqid, Fuid, Ftrue_name, Fcre_id_orig, Fphone, Fmobile, Fstate, Flstate, "
        " Fcreate_time, Fmodify_time,Facct_type,Fchannel_id,Fopenid,Fcft_auth_type,Ffund_auth_type,Fstandby2,Fstandby4,Fstandby11 ) values "
        "(%d, '%s', '%s', '%s', %d, '%s', '%s', '%s', '%s', %d, %d, '%s', '%s', %d, '%s', '%s', %d, %d,'%s',%d, '%s')",
        pstRecord->Fcre_type, 
        1 == gPtrConfig->m_AppCfg.db_encode_switch?escapeString(strEncodeCreId).c_str():escapeString(pstRecord->Fcre_id).c_str(),
        escapeString(pstRecord->Ftrade_id).c_str(), escapeString(pstRecord->Fqqid).c_str(), pstRecord->Fuid,
        escapeString(pstRecord->Ftrue_name).c_str(), 
        1 == gPtrConfig->m_AppCfg.db_encode_switch?escapeString(strEncodeCreIdOrig).c_str():escapeString(pstRecord->Fcre_id_orig).c_str(),
        1 == gPtrConfig->m_AppCfg.db_encode_switch?escapeString(strEncodeFphone).c_str():escapeString(pstRecord->Fphone).c_str(),
        1 == gPtrConfig->m_AppCfg.db_encode_switch?escapeString(strEncodeFmobile).c_str():escapeString(pstRecord->Fmobile).c_str(),        
        pstRecord->Fstate, pstRecord->Flstate, pstRecord->Fcreate_time, pstRecord->Fmodify_time,
        pstRecord->Facct_type, escapeString(pstRecord->Fchannel_id).c_str(), escapeString(pstRecord->Fopenid).c_str(),
        pstRecord->Fcft_auth_type, pstRecord->Ffund_auth_type, escapeString(pstRecord->Fassess_modify_time).c_str(),pstRecord->Fassess_risk_type,
        genSign("t_fund_bind", *pstRecord).c_str());
        
    mysql->Query(szSqlStr, iLen);

    // 判断影响行数
    if(1 != mysql->AffectedRows())
    {
        gPtrAppLog->error("insert fundbind affected rows[%d] error, sql[%s]", mysql->AffectedRows(), szSqlStr); 
        throw CException(ERR_DB_AFFECT_MULTIROWS, "insert fundbind affected rows error! ", __FILE__, __LINE__);
    }
}

/**
 * 新增基金交易流水，基金交易(订单)表
 */
void InsertTradeFund(CMySQL* mysql, ST_TRADE_FUND* pstRecord)
{
    char    szSqlStr[MAX_SQL_LEN*2+1] = {0};

    // 转义单号
    string sEscapelistId = escapeString(pstRecord->Flistid);
    
    int iLen = snprintf(szSqlStr, sizeof(szSqlStr), 
        "insert into fund_db_%02d.t_trade_fund_%d "
        "(Flistid, Fspid, Fcoding, Ftrade_id, Fuid, Ffund_name, Ffund_code, Fpur_type, Ftotal_fee,"
        "Fbank_type, Fcard_no, Fstate, Flstate, Ftrade_date, Ffund_value, Ffund_vdate, Ffund_type, Fnotify_url, "
        "Frela_listid, Fdrawid, Ffetchid, Fcft_timestamp, Fcreate_time, Fmodify_time, Fstandby1, Fcft_trans_id, "
        "Fcft_charge_ctrl_id,Fsp_fetch_id,Fcft_bank_billno, Fsub_trans_id, Fcur_type, Facc_time, Fpurpose,Fchannel_id, " 
        "Floading_type,Fstandby4,Fend_date,Freal_redem_amt, Fopt_type, Fclose_listid,Fsign,Fpay_channel) values "
        "('%s', '%s', '%s', '%s', %d, '%s', '%s', %d, %zd, %d, '%s', %d, %d,'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', " 
        " %d, '%s', '%s', %d, '%s', '%s', '%s', '%s', '%s',%d, '%s',%d, '%s',%d, '%s', '%s', %zd, %d, %zd, '%s', %d)",
        Sdb2(sEscapelistId.c_str()), Stb2(sEscapelistId.c_str()), sEscapelistId.c_str(), 
        escapeString(pstRecord->Fspid).c_str(), escapeString(pstRecord->Fcoding).c_str(), escapeString(pstRecord->Ftrade_id).c_str(), pstRecord->Fuid, 
        escapeString(pstRecord->Ffund_name).c_str(), escapeString(pstRecord->Ffund_code).c_str(), 
        pstRecord->Fpur_type, pstRecord->Ftotal_fee, pstRecord->Fbank_type, escapeString(pstRecord->Fcard_no).c_str(), 
        pstRecord->Fstate, pstRecord->Flstate, escapeString(pstRecord->Ftrade_date).c_str(), escapeString(pstRecord->Ffund_value).c_str(), escapeString(pstRecord->Ffund_vdate).c_str(), 
        escapeString(pstRecord->Ffund_type).c_str(), escapeString(pstRecord->Fnotify_url).c_str(), 
        escapeString(pstRecord->Frela_listid).c_str(), escapeString(pstRecord->Fdrawid).c_str(), escapeString(pstRecord->Ffetchid).c_str(), pstRecord->Fcft_timestamp, 
        escapeString(pstRecord->Fcreate_time).c_str(), escapeString(pstRecord->Fmodify_time).c_str(), pstRecord->Fstandby1, escapeString(pstRecord->Fcft_trans_id).c_str(), 
        escapeString(pstRecord->Fcft_charge_ctrl_id).c_str(),escapeString(pstRecord->Fsp_fetch_id).c_str(),escapeString(pstRecord->Fcft_bank_billno).c_str(), escapeString(pstRecord->Fsub_trans_id).c_str(), 
        pstRecord->Fcur_type, pstRecord->Facc_time, pstRecord->Fpurpose,escapeString(pstRecord->Fchannel_id).c_str(),pstRecord->Floading_type,escapeString(pstRecord->Fcoupon_id).c_str(),
        escapeString(pstRecord->Fend_date).c_str(),pstRecord->Freal_redem_amt, pstRecord->Fopt_type, pstRecord->Fclose_listid,
        genSign("t_trade_fund", *pstRecord).c_str(), pstRecord->Fpay_channel);

    mysql->Query(szSqlStr, iLen);

    // 判断影响行数
    if(1 != mysql->AffectedRows())
    {
        gPtrAppLog->error("insert tradefund affected rows[%d] error, sql[%s]", mysql->AffectedRows(), szSqlStr); 
        throw CException(ERR_DB_AFFECT_MULTIROWS, "insert tradefund affected rows error! ", __FILE__, __LINE__);
    }
}


/**
 * 新增基金交易变更流水，基金交易(订单)表变更流水表
 */
void InsertTradeFundLog(CMySQL* mysql, ST_TRADE_FUND* pstRecord, const string &systime)
{
    char    szSqlStr[MAX_SQL_LEN*2+1] = {0};

    // 转义单号
    string sEscapelistId = escapeString(pstRecord->Flistid);

    int iLen = snprintf(szSqlStr, sizeof(szSqlStr), 
        "insert into fund_db_%02d.t_trade_fund_log_%d "
        "(Flistid, Fspid, Fcoding, Ftrade_id, Fuid, Ffund_name, Ffund_code, Fpur_type, Ftotal_fee,"
        "Fbank_type, Fcard_no, Fstate, Flstate, Ftrade_date, Ffund_value, Ffund_vdate, Ffund_type, Fnotify_url, "
        "Frela_listid, Fdrawid, Ffetchid, Fcft_timestamp, Fcreate_time, Fmodify_time, Fstandby1, Fcft_trans_id, "
        "Fcft_charge_ctrl_id,Fsp_fetch_id,Fcft_bank_billno, Fsub_trans_id, Fcur_type, Facc_time, Fpurpose,Fchannel_id,Floading_type) values "
        "('%s', '%s', '%s', '%s', %d, '%s', '%s', %d, %zd, %d, '%s', %d, %d,'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d, '%s', '%s', %d, '%s', '%s', '%s', '%s', '%s',%d, '%s',%d, '%s',%d)", 
        Sdb2(sEscapelistId.c_str()), Stb2(sEscapelistId.c_str()), sEscapelistId.c_str(), 
        escapeString(pstRecord->Fspid).c_str(), escapeString(pstRecord->Fcoding).c_str(), escapeString(pstRecord->Ftrade_id).c_str(), pstRecord->Fuid, 
        escapeString(pstRecord->Ffund_name).c_str(), escapeString(pstRecord->Ffund_code).c_str(), 
        pstRecord->Fpur_type, pstRecord->Ftotal_fee, pstRecord->Fbank_type, escapeString(pstRecord->Fcard_no).c_str(), 
        pstRecord->Fstate, pstRecord->Flstate, escapeString(pstRecord->Ftrade_date).c_str(), escapeString(pstRecord->Ffund_value).c_str(), escapeString(pstRecord->Ffund_vdate).c_str(), 
        escapeString(pstRecord->Ffund_type).c_str(), escapeString(pstRecord->Fnotify_url).c_str(), 
        escapeString(pstRecord->Frela_listid).c_str(), escapeString(pstRecord->Fdrawid).c_str(), escapeString(pstRecord->Ffetchid).c_str(), pstRecord->Fcft_timestamp, 
        escapeString(systime).c_str(), escapeString(systime).c_str(), pstRecord->Fstandby1, escapeString(pstRecord->Fcft_trans_id).c_str(), 
        escapeString(pstRecord->Fcft_charge_ctrl_id).c_str(),escapeString(pstRecord->Fsp_fetch_id).c_str(),escapeString(pstRecord->Fcft_bank_billno).c_str(), escapeString(pstRecord->Fsub_trans_id).c_str(), 
        pstRecord->Fcur_type, pstRecord->Facc_time,pstRecord->Fpurpose,escapeString(pstRecord->Fchannel_id).c_str(),pstRecord->Floading_type);

    mysql->Query(szSqlStr, iLen);

    // 判断影响行数
    if(1 != mysql->AffectedRows())
    {
        gPtrAppLog->error("insert tradefund affected rows[%d] error, sql[%s]", mysql->AffectedRows(), szSqlStr); 
        throw CException(ERR_DB_AFFECT_MULTIROWS, "insert tradefund affected rows error! ", __FILE__, __LINE__);
    }
}



/**
 * 新增基金交易流水，基金交易(用户)表
 */
void InsertTradeUserFund(CMySQL* mysql, ST_TRADE_FUND* pstRecord)
{
    char    szSqlStr[MAX_SQL_LEN*2+1] = {0};

    int iLen = snprintf(szSqlStr, sizeof(szSqlStr), 
        "insert into fund_db_%02d.t_trade_user_fund_%d "
        "(Flistid, Fspid, Fcoding, Ftrade_id, Fuid, Ffund_name, Ffund_code, Fpur_type, Ftotal_fee,"
        "Fbank_type, Fcard_no, Fstate, Flstate, Ftrade_date, Ffund_value, Ffund_vdate, Ffund_type, Fnotify_url, "
        "Frela_listid, Fdrawid, Ffetchid, Fcft_timestamp, Fcreate_time, Fmodify_time,Fcft_trans_id,"
        "Fcft_charge_ctrl_id,Fsp_fetch_id,Fcft_bank_billno, Fsub_trans_id, Fcur_type, Facc_time, Fpurpose,Fchannel_id,"
        "Floading_type,Fopt_type,Fclose_listid,Freal_redem_amt, Fend_date, Fsign, Fpay_channel ) values "
        "('%s', '%s', '%s', '%s', %d, '%s', '%s', %d, %zd, %d, '%s', %d, %d,'%s', '%s', '%s', '%s', '%s', '%s',"
        "'%s', '%s', %d, '%s', '%s', '%s', '%s', '%s','%s','%s',%d, '%s',%d, '%s',%d,%d, %zd,%zd,  '%s', '%s', %d)", 
        Sdb1(pstRecord->Fuid), Stb1(pstRecord->Fuid), escapeString(pstRecord->Flistid).c_str(), 
        escapeString(pstRecord->Fspid).c_str(), escapeString(pstRecord->Fcoding).c_str(), escapeString(pstRecord->Ftrade_id).c_str(), pstRecord->Fuid, 
        escapeString(pstRecord->Ffund_name).c_str(), escapeString(pstRecord->Ffund_code).c_str(), 
        pstRecord->Fpur_type, pstRecord->Ftotal_fee, pstRecord->Fbank_type, escapeString(pstRecord->Fcard_no).c_str(), 
        pstRecord->Fstate, pstRecord->Flstate, escapeString(pstRecord->Ftrade_date).c_str(), escapeString(pstRecord->Ffund_value).c_str(), escapeString(pstRecord->Ffund_vdate).c_str(), 
        escapeString(pstRecord->Ffund_type).c_str(), escapeString(pstRecord->Fnotify_url).c_str(), 
        escapeString(pstRecord->Frela_listid).c_str(), escapeString(pstRecord->Fdrawid).c_str(), escapeString(pstRecord->Ffetchid).c_str(), pstRecord->Fcft_timestamp, 
        escapeString(pstRecord->Fcreate_time).c_str(), escapeString(pstRecord->Fmodify_time).c_str(), escapeString(pstRecord->Fcft_trans_id).c_str(), 
        escapeString(pstRecord->Fcft_charge_ctrl_id).c_str(),escapeString(pstRecord->Fsp_fetch_id).c_str(),escapeString(pstRecord->Fcft_bank_billno).c_str(), escapeString(pstRecord->Fsub_trans_id).c_str(), 
        pstRecord->Fcur_type, pstRecord->Facc_time, pstRecord->Fpurpose,escapeString(pstRecord->Fchannel_id).c_str(),pstRecord->Floading_type,pstRecord->Fopt_type,pstRecord->Fclose_listid,
        pstRecord->Freal_redem_amt, escapeString(pstRecord->Fend_date).c_str(), 
        genSign("t_trade_user_fund", *pstRecord).c_str(),  pstRecord->Fpay_channel);

    mysql->Query(szSqlStr, iLen);

    // 判断影响行数
    if(1 != mysql->AffectedRows())
    {
        gPtrAppLog->error("insert tradeuserfund affected rows[%d] error, sql[%s]", mysql->AffectedRows(), szSqlStr); 
        throw CException(ERR_DB_AFFECT_MULTIROWS, "insert tradeuserfund affected rows error! ", __FILE__, __LINE__);
    }
}

int UpdateTable(CMySQL* db_conn, const string &tb_name, const stringstream &ss_cond, const map<string, string> &kv_map)
{
    stringstream ss_sql;
    
    // 生成sql语句
    ss_sql << "update " << tb_name << " set ";

    // 生成更新的字段列表
    for (map<string, string>::const_iterator iter = kv_map.begin(); iter != kv_map.end(); ++iter)
    {
        if (iter != kv_map.begin())
        {
            ss_sql << ",";
        }

        ss_sql << iter->first << "='" << escapeString(iter->second) << "'";
    }

    ss_sql << " where " << ss_cond.str();
	gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,ss_sql.str().c_str() );

    // 执行sql
    db_conn->Query(ss_sql.str().c_str(), ss_sql.str().size());
    return db_conn->AffectedRows();
}

int QueryFundTime(CMySQL *hsql, list<ST_FUND_TIME> &l_res, const char *sp_id/* = NULL*/, bool is_lock/* = false*/)
{
    if(!hsql)
    {
        gPtrAppLog->error("query fund time: sql handle is null");
        return -1;
    }
    
    char    szSqlStr[MAX_SQL_LEN*2+1] = {0};
    MYSQL_RES*    pResult = NULL;
    MYSQL_ROW    row;

    string now_time(32, '\0');
    GetTimeNow(&now_time[0]);

    string strLock = is_lock ? "FOR UPDATE" : "";

    string cond;
    if(sp_id)
    {
        cond = "and Fspid='" + escapeString(sp_id) + "'";
    }
    else
        cond = "";

    int iLen = snprintf(szSqlStr, sizeof(szSqlStr), 
        "select Fspid, Fpur_type, Fdead_line, Fstime, Flstate, Fdead_line_last, Fcreate_time, Fmodify_time, Fstandby1, Fstandby2, Fstandby3, Fstandby4, Fstandby5 "
        "from fund_db.t_fund_time "
        "where Flstate=1 and Fpur_type=2 and Fetime>'%s' and Fstime<'%s' %s %s", 
        now_time.c_str(), now_time.c_str(), cond.c_str(), strLock.c_str());

    hsql->Query(szSqlStr, iLen);

    pResult = hsql->FetchResult();

    int row_num = mysql_num_rows(pResult);
    if(0 == row_num)
    {
        mysql_free_result(pResult);
        return 0;
    }

    while((row = mysql_fetch_row(pResult)))
    {
        ST_FUND_TIME stRecord;
        memset(&stRecord, 0, sizeof(ST_FUND_TIME));

        strncpy(stRecord.Fspid, ValiStr(row[0]), sizeof(stRecord.Fspid)-1);
        stRecord.Fpur_type= atoi(ValiStr(row[1]));
        strncpy(stRecord.Fdead_line, ValiStr(row[2]), sizeof(stRecord.Fdead_line)-1);
        strncpy(stRecord.Fstime, ValiStr(row[3]), sizeof(stRecord.Fstime)-1);
        stRecord.Flstate= atoi(ValiStr(row[4]));
        strncpy(stRecord.Fdead_line_last, ValiStr(row[5]), sizeof(stRecord.Fdead_line_last)-1);
        strncpy(stRecord.Fcreate_time, ValiStr(row[6]), sizeof(stRecord.Fcreate_time)-1);
        strncpy(stRecord.Fmodify_time, ValiStr(row[7]), sizeof(stRecord.Fmodify_time)-1);
        stRecord.Fstandby1= atoi(ValiStr(row[8]));
        stRecord.Fstandby2= atoi(ValiStr(row[9]));
        strncpy(stRecord.Fstandby3, ValiStr(row[10]), sizeof(stRecord.Fstandby3)-1);
        strncpy(stRecord.Fstandby4, ValiStr(row[11]), sizeof(stRecord.Fstandby4)-1);
        strncpy(stRecord.Fstandby5, ValiStr(row[12]), sizeof(stRecord.Fstandby5)-1);

        l_res.push_back(stRecord);
    }

    mysql_free_result(pResult);
    return row_num;
}


/*
 * 获取交易描述符
 */
string getTradeDesc(int pur_type)
{
    static const char * desc_str[] = {"申购", "认购", "定投",
            "赎回", "撤销", "分红", "认申购失败", "比例确认退款"};

    if (pur_type < 1 || pur_type > 8)
    {
        return "";
    }

    return desc_str[pur_type - 1];
}

/*
 * 更新交易表，同时记录变更流水
 * 参数说明: ST_TRADE_FUND& pstDbRecord --- 当前最新的交易记录表数据，必须包含参与签名的字段如下:
                 Flistid、Fpur_type、Fspid、Ftrade_id、Fuid、Ftotal_fee、Fstate为最新DB值用于计算sign签名，
                 其他字段值可暂不关心
 */
void UpdateFundTrade(CMySQL* mysql, ST_TRADE_FUND& pstRecord, ST_TRADE_FUND& pstDbRecord, const string &systime, bool updateUserTable)
{
    stringstream tb_name;
    stringstream ss_cond;
    map<string, string> kv_map;

	string listid = escapeString(pstRecord.Flistid);
    bool bNeedSetDbSign = false;

    tb_name << "fund_db_" << listid.substr(listid.size() - 2);
    tb_name << ".t_trade_fund_" << listid.substr(listid.size() - 3, 1);

    // 设置需要更新的字段
	kv_map["Fspe_tag"] = toString(pstRecord.Fspe_tag);    
    kv_map["Fmodify_time"] = systime;
    
	if(pstRecord.Fstate!= 0)
	{
        kv_map["Fstate"] = toString(pstRecord.Fstate);
        bNeedSetDbSign = true;
    	}
	if(!(0 == strcmp("", pstRecord.Fcoding)))
	{
		kv_map["Fcoding"] = pstRecord.Fcoding;
	}
	if(!(0 == strcmp("", pstRecord.Fcft_trans_id)))
	{
		kv_map["Fcft_trans_id"] = pstRecord.Fcft_trans_id;
	}

	if(!(0 == strcmp("", pstRecord.Facc_time)))
	{
		kv_map["Facc_time"] = pstRecord.Facc_time;
	}
	if(!(0 == strcmp("", pstRecord.Fsub_trans_id)))
	{
		kv_map["Fsub_trans_id"] = pstRecord.Fsub_trans_id;
	}
	if(!(0 == strcmp("", pstRecord.Ftrade_id)))
	{
        kv_map["Ftrade_id"] = pstRecord.Ftrade_id;//预支付订单没有该值，开户完成进行在申购请求时会补填信息
        bNeedSetDbSign = true;
	}
	if(pstRecord.Fuid != 0)
	{
        kv_map["Fuid"] = toString(pstRecord.Fuid);
        bNeedSetDbSign = true;
	}
	if(pstRecord.Fpurpose!= 0)
	{
		kv_map["Fpurpose"] = toString(pstRecord.Fpurpose);
	}
	if(!(0 == strcmp("", pstRecord.Ftrade_date)))
	{
		kv_map["Ftrade_date"] = pstRecord.Ftrade_date;
	}
	if(!(0 == strcmp("", pstRecord.Ffund_vdate)))
	{
		kv_map["Ffund_vdate"] = pstRecord.Ffund_vdate;
	}
	if(!(0 == strcmp("", pstRecord.Fmemo)))
	{
		kv_map["Fmemo"] = pstRecord.Fmemo;
	}
	if(pstRecord.Floading_type!= 0)
	{
		kv_map["Floading_type"] = toString(pstRecord.Floading_type);
	}
	if(pstRecord.Fbank_type!= 0)
	{
		kv_map["Fbank_type"] = toString(pstRecord.Fbank_type);
	}
	if(pstRecord.Fcard_no[0]!= 0)
	{
		kv_map["Fcard_no"] = pstRecord.Fcard_no;
	}
	if(pstRecord.Freal_redem_amt!= 0)
	{
		kv_map["Freal_redem_amt"] = toString(pstRecord.Freal_redem_amt);
	}
	if(pstRecord.Fclose_listid!= 0)
	{
		kv_map["Fclose_listid"] = toString(pstRecord.Fclose_listid);
	}
	if(!(0 == strcmp("", pstRecord.Fend_date)))
	{
		kv_map["Fend_date"] = pstRecord.Fend_date;
	}
	if(pstRecord.Freal_redem_amt!=0)
	{
		kv_map["Freal_redem_amt"] = toString(pstRecord.Freal_redem_amt);
	}
       if(pstRecord.Frefund_type!=0)
	{
		kv_map["Frefund_type"] = toString(pstRecord.Frefund_type);
	}
	if(pstRecord.Fcharge_fee!=0)
	{
		kv_map["Fcharge_fee"] = toString(pstRecord.Fcharge_fee);
	}	
	if(0!=strcmp(pstRecord.Ffund_value,""))
	{
		kv_map["Ffund_value"] = pstRecord.Ffund_value;
	}	
	if(pstRecord.Ffetch_result!=0)
	{
	       kv_map["Ffetch_result"] = toString(pstRecord.Ffetch_result);
	}
	if(!(0 == strcmp("", pstRecord.Ffetch_arrival_time)))
	{
		kv_map["Ffetch_arrival_time"] = pstRecord.Ffetch_arrival_time;
	}
	if(!(0 == strcmp("", pstRecord.Fcharge_type)))
	{
		kv_map["Fcharge_type"] = pstRecord.Fcharge_type;
	}
	if(pstRecord.Frefund_reason!=0)
	{
	  kv_map["Frefund_reason"] = toString(pstRecord.Frefund_reason);
	}
	
	if( bNeedSetDbSign ){
        kv_map["Fsign"] = genMergeSign("t_trade_fund", pstRecord, pstDbRecord);
    }
     
    ss_cond << "Flistid='" << listid <<"' ";
	ss_cond << " AND Fpur_type=" << toString(pstRecord.Fpur_type);

    // 执行更新数据表操作
    int affect_row = UpdateTable(mysql, tb_name.str(), ss_cond, kv_map);
    if (affect_row != 1)
    {
		gPtrAppLog->error("listid[%s], affect_row[%d]", listid.c_str(),affect_row);
        throw EXCEPTION(ERR_DB_AFFECT_MULTIROWS, "update affect rows error");
    }
	if(updateUserTable)
	{
		char    user_tb_name[MAX_SQL_LEN*2+1] = {0};
		snprintf(user_tb_name, sizeof(user_tb_name), "fund_db_%02d.t_trade_user_fund_%d ", Sdb1(pstRecord.Fuid), Stb1(pstRecord.Fuid));

		// 执行更新数据表操作
		affect_row = UpdateTable(mysql, user_tb_name, ss_cond, kv_map);
		if (affect_row != 1)
		{
			gPtrAppLog->error("listid[%s], affect_row[%d]", listid.c_str(),affect_row);
			throw EXCEPTION(ERR_DB_AFFECT_MULTIROWS, "update affect rows error");
		}
	}

	//记录变更流水,减少压力不再记录
	//InsertTradeFundLog(mysql, &pstRecord, systime);

	setTradeRecordsToKV(mysql,pstRecord);

       //收益到账日期显示需求
       //对于申购成功和赎回成功的单发送异步通知消息给fund_trade_result_service统计交易日实时余额
       if (pstRecord.Fpur_type != PURTYPE_TRANSFER_REDEEM)
       {
           notifyTradeSuccInfo(pstRecord);
       }
       

}

//收益到账日期显示需求
//对于申购成功和赎回成功的单发送异步通知消息给fund_trade_result_service统计交易日实时余额
void notifyTradeSuccInfo(ST_TRADE_FUND& pstRecord)
{
    if ((pstRecord.Fstate != PURCHASE_SUC
        && pstRecord.Fstate != REDEM_FINISH))
    { 
        //只有申购成功和赎回成功的单才通知给T日实时余额计算服务
        return;
    }

    try
    {
        map<string,string> mapReqPara;
        mapReqPara["listid"]= pstRecord.Flistid;
        mapReqPara["trade_id"]= pstRecord.Ftrade_id;
        mapReqPara["total_fee"]= toString(pstRecord.Ftotal_fee);
        mapReqPara["state"]= toString(pstRecord.Fstate);
        mapReqPara["acc_time"]= pstRecord.Facc_time;
        string reqPara;
        Tools::MapToStr(mapReqPara,reqPara);
    
        gPtrFundFetchRpc->excute_asyn("fund_trade_result_service", reqPara.c_str(), reqPara.length());
    }
    catch(CException &e)
    {
         //只打印日志不要抛出异常
         gPtrAppLog->error("notifyTradeSuccInfo exception: code=%d,errinfo=%s",e.error(),e.what());
    }
}


/*
 * 更新t_fund_bind 的fstandy1值
 */
 
void UpdateFundBindAssetLimit(CMySQL* mysql, ST_FUND_BIND& data)
{
 char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db.t_fund_bind SET "
                    " Fstandby1= %d, "
                    " Fmodify_time='%s' "
                    " WHERE "
                    " Ftrade_id='%s'", 
                    data.Fasset_limit_lev,
                    mysql->EscapeStr(data.Fmodify_time).c_str(),
                    //--------where条件--------
                    mysql->EscapeStr(data.Ftrade_id).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    mysql->Query(szSql, iLen);
    // 判断影响行数是否唯一
    if (mysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
     }
}

void UpdateFundBindFlstate(CMySQL* mysql, ST_FUND_BIND& data)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db.t_fund_bind SET "
                    " Flstate= %d, "
                    " Fstandby5= %d,"
                    " Fmodify_time='%s' "
                    " WHERE "
                    " Ftrade_id='%s'", 
                    data.Flstate,
                    data.Ffrozen_channle,
                    mysql->EscapeStr(data.Fmodify_time).c_str(),
                    //--------where条件--------
                    mysql->EscapeStr(data.Ftrade_id).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    mysql->Query(szSql, iLen);
    // 判断影响行数是否唯一
    if (mysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}

/*
 * 更新t_fund_bind表，同时记录变更流水
 * 参数说明: ST_FUND_BIND& dbFundBind --- 当前最新的交易记录表数据，且必须包含参与签名的字段如下:
                                           Ftrade_id、Fcre_id、Fqqid、Fuid、Fmobile、Fopenid为最新DB值用于计算sign签名，
                                           其他字段值可暂不关心
 */
void UpdateFundBind(CMySQL* mysql, ST_FUND_BIND& fundBind, ST_FUND_BIND& dbFundBind, const string &systime)
{
    bool bNeedSetDbSign = false;
        
    string tb_name;
    map<string, string> kv_map;
    
    tb_name = "fund_db.t_fund_bind";

    // 设置需要更新的字段
    if(!(0 == strcmp("", fundBind.Fmobile)))
	{
        kv_map["Fmobile"] = fundBind.Fmobile;
        bNeedSetDbSign = true;
	}

    if(!(0 == strcmp("", fundBind.Fphone)))
    {
    	    kv_map["Fphone"] = fundBind.Fphone;
    }

    if(!(0 == strcmp("", fundBind.Femail)))
	{
		kv_map["Fstandby3"] = fundBind.Femail;
	}

    if(!(0 == strcmp("", fundBind.Faddress)))
	{
		kv_map["Fstandby12"] = fundBind.Faddress;
	}

	if(fundBind.Fuid != 0)
	{
        kv_map["Fuid"] = toString(fundBind.Fuid);
        bNeedSetDbSign = true;
	}

    if (fundBind.Fasset_limit_lev>0)
    {   
        kv_map["Fstandby1"] = toString(fundBind.Fasset_limit_lev);
    }

    if (fundBind.Fassess_risk_type>0)
    {   
        kv_map["Fstandby4"] = toString(fundBind.Fassess_risk_type);
    }

    if(!(0 == strcmp("",fundBind.Fassess_modify_time)))
    {
	    kv_map["Fstandby2"] = escapeString(fundBind.Fassess_modify_time);
    }
	
    kv_map["Fmodify_time"] = systime;
    
	if(bNeedSetDbSign){
        kv_map["Fstandby11"] = genMergeSign("t_fund_bind", fundBind, dbFundBind);
    }

    stringstream ss_cond;
    ss_cond << "Ftrade_id='" << escapeString(fundBind.Ftrade_id) << "'";
    
    // 执行更新数据表操作
    int affect_row = UpdateTable(mysql, tb_name, ss_cond, kv_map);
    if (affect_row != 1)
    {
        throw EXCEPTION(ERR_DB_AFFECT_MULTIROWS, "update affect rows error");
    }
}

/**
 * 根据db数据，将需要写入CKV的用户基本信息拼接成value串
 * @param fundBind 用户基本信息
 * @param value 返回的拼接串
 * @return 0-成功，其它-失败
 */
int packUsrFundBindCkvValue(const ST_FUND_BIND& fundBind, string &value)
{
	CParams kvReqSet;

    //设置要写入ckv的数据
	kvReqSet.setParam("Fuin",fundBind.Fqqid);
	kvReqSet.setParam("Fcre_type",fundBind.Fcre_type);
	kvReqSet.setParam("Ftrade_id",fundBind.Ftrade_id);
	kvReqSet.setParam("Fuid",fundBind.Fuid);
	kvReqSet.setParam("Ftrue_name",fundBind.Ftrue_name);
	kvReqSet.setParam("Fstate",fundBind.Fstate);
    kvReqSet.setParam("Flstate",fundBind.Flstate);
	kvReqSet.setParam("Fcreate_time",fundBind.Fcreate_time);
	kvReqSet.setParam("Facct_type",fundBind.Facct_type);
	kvReqSet.setParam("Fopenid",fundBind.Fopenid);
	kvReqSet.setParam("Fchannel_id",fundBind.Fchannel_id);
	kvReqSet.setParam("Fasset_limit_lev",fundBind.Fasset_limit_lev);
	kvReqSet.setParam("Fassess_risk_type",fundBind.Fassess_risk_type);
	kvReqSet.setParam("Fassess_time",fundBind.Fassess_modify_time);
	kvReqSet.setParam("Femail",fundBind.Femail);
	kvReqSet.setParam("Faddress",fundBind.Faddress);
	// 姓名CKV打码
	char name[64]={0};
	hideName(fundBind.Ftrue_name,name);
	kvReqSet.setParam("Ftrue_name",name);

	// 证件打码
	char creId[32]={0};
	hideCreId(fundBind.Fcre_id,fundBind.Fcre_type,creId);
	kvReqSet.setParam("Fcre_id",creId);
	
	// 从证件中获取性别
	int sex = getSexFromCreId(fundBind.Fcre_id,fundBind.Fcre_type);
	kvReqSet.setParam("Fsex",sex);
	
	// 手机号打码:显示前3后2,隐藏中间6位
	char phone[32]={0};
	hideChar(fundBind.Fphone,phone,3,6);
	kvReqSet.setParam("Fphone",phone);
	
    value = kvReqSet.pack();

    return 0;
}

/**
*设置cache
*/
bool setFundBindToKV(CMySQL* mysql, ST_FUND_BIND& fundBind, bool needQuery)
{
	string key = toString(fundBind.Fqqid);
    
    if(needQuery)
    {
		bool ret = QueryFundBindByUin(mysql, fundBind.Fqqid, &fundBind, false);
        //如果查询不到数据则删除ckv，如果用户注销后不删除则会导致用户重新进来用老的信息去下单
        if (false == ret) {
            if(gCkvSvrOperator->del(CKV_KEY_UIN, key))
                return false;
            else
                return true;
        }
    }
    
    string szValue;
    packUsrFundBindCkvValue(fundBind, szValue);

    //将szValue写入ckv
    if(gCkvSvrOperator->set(CKV_KEY_UIN, key, szValue))
    {
		return false;
    }
	else
	{
		return true;
	}
}


bool delFundbindToKV(string uin)
{
    string key = uin;

    gPtrAppLog->debug("delFundbindToKV uin=%s",uin.c_str());

    //将szValue写入ckv
    if(gCkvSvrOperator->del(CKV_KEY_UIN, key))
    {
        return false;
    }
    else
    {
        return true;
    }
}

/**
 * 组装用户交易记录的CKV值
 * @param list 用户交易记录
 * @param value 组装后的ckv值
 * @return 0-成功，其它-失败
 */
int packTradeRecordsCkvValue(const vector<ST_TRADE_FUND> &list, string &value)
{
    CParams kvReqSet;
	char szParaName[64] = {0};

    //设置要修改的数据szValue
    vector<ST_TRADE_FUND>::size_type i= 0;
    for(i= 0; i != list.size() && i < CACHE_USR_FUND_TRADE_MAX_NUM; ++i)
    {
		const ST_TRADE_FUND &tradeFund = list[i];
		
		snprintf(szParaName, sizeof(szParaName), "Flistid_%zd", i);
		kvReqSet.setParam(szParaName, tradeFund.Flistid);
		
		snprintf(szParaName, sizeof(szParaName), "Ftrade_id_%zd", i);
		kvReqSet.setParam(szParaName, tradeFund.Ftrade_id);
		
		snprintf(szParaName, sizeof(szParaName), "Fspid_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fspid);
		
		snprintf(szParaName, sizeof(szParaName), "Fcoding_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fcoding);
		
		snprintf(szParaName, sizeof(szParaName), "Fuid_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fuid);
		
		snprintf(szParaName, sizeof(szParaName), "Ffund_code_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Ffund_code);
		
		snprintf(szParaName, sizeof(szParaName), "Ffund_name_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Ffund_name);
		
		snprintf(szParaName, sizeof(szParaName), "Fpur_type_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fpur_type);
		
		snprintf(szParaName, sizeof(szParaName), "Ftotal_fee_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Ftotal_fee);
		
		snprintf(szParaName, sizeof(szParaName), "Fbank_type_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fbank_type);

		snprintf(szParaName, sizeof(szParaName), "Fstate_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fstate);
		
		snprintf(szParaName, sizeof(szParaName), "Fpurpose_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fpurpose);
		
		snprintf(szParaName, sizeof(szParaName), "Fcreate_time_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fcreate_time);
		
		snprintf(szParaName, sizeof(szParaName), "Facc_time_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Facc_time);

		snprintf(szParaName, sizeof(szParaName), "Ftrade_date_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Ftrade_date);

		snprintf(szParaName, sizeof(szParaName), "Ffund_vdate_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Ffund_vdate);

		snprintf(szParaName, sizeof(szParaName), "Floading_type_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Floading_type);

		snprintf(szParaName, sizeof(szParaName), "Fchannel_id_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fchannel_id);

		snprintf(szParaName, sizeof(szParaName), "Fclose_listid_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fclose_listid);

		snprintf(szParaName, sizeof(szParaName), "Fopt_type_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fopt_type);

		snprintf(szParaName, sizeof(szParaName), "Freal_redem_amt_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Freal_redem_amt);

		snprintf(szParaName, sizeof(szParaName), "Fend_date_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fend_date);

              snprintf(szParaName, sizeof(szParaName), "Frefund_type_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Frefund_type);
		
              snprintf(szParaName, sizeof(szParaName), "Ffetch_arrival_time_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Ffetch_arrival_time);

		 snprintf(szParaName, sizeof(szParaName), "Ffetch_result_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Ffetch_result);

		 snprintf(szParaName, sizeof(szParaName), "Fcharge_type_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fcharge_type);

		 snprintf(szParaName, sizeof(szParaName), "Fcharge_fee_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fcharge_fee);

		 snprintf(szParaName, sizeof(szParaName), "Fspe_tag_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fspe_tag);

         snprintf(szParaName, sizeof(szParaName), "Fpay_channel_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fpay_channel);
    }

	kvReqSet.setParam("total_num",
(int)i);
	if(list.size() >= CACHE_USR_FUND_TRADE_MAX_NUM)
	{
		kvReqSet.setParam("isMore", "1"); //标识有更多数据在数据库中
	}
	else
	{
		kvReqSet.setParam("isMore", "0");
	}
    
    value = kvReqSet.pack();

    return 0;
}


/**
*设置cache
*/
bool setTradeRecordsToKV(CMySQL* mysql, ST_TRADE_FUND& trade_fund)
{
	if(trade_fund.Fstate>=TRADE_STATE_SIZE||trade_fund.Fstate<=0)
	{
		return false;
	}
	// 支付单在支付成功之后更新CKV
	// 赎回单在赎回通知基金公司成功之后更新CKV
	// 赎回单发起赎回也更新CKV
	if(PURCHASE_STATE_ORDER[trade_fund.Fstate]<PURCHASE_STATE_ORDER[PAY_OK]&&
		REDEEM_STATE_ORDER[trade_fund.Fstate]<REDEEM_STATE_ORDER[REDEEM_INFO_SUC]&&trade_fund.Fstate!=REDEM_ININ)
	{
		return false;
	}

    /* 检查uid是否有效 */
    if (trade_fund.Fuid <= 0) {
        TRACE_ERROR("fund trade record Fuid<=0");
        return false;
    }

    /* 在调该接口时trade_id应必须传入，如果trade_id为空则根据uid查询trade_id以做兼容处理,否则无法更新ckv */
    string trade_id = trade_fund.Ftrade_id;
    if (trade_id.empty()) {
        TRACE_DEBUG("[setTradeRecordsToKV]trade_id is empty. query trade_id by uid=%d", 
            trade_fund.Fuid);
        ST_FUND_BIND fund_bind;
        QueryFundBindByUid(mysql, trade_fund.Fuid, &fund_bind, false);
        SCPY(trade_fund.Ftrade_id, fund_bind.Ftrade_id);
    }

    /* 检查trade_id是否为空 */
    if (strnlen(trade_fund.Ftrade_id, sizeof(trade_fund.Ftrade_id)) <= 0) {
        TRACE_ERROR("fund trade record Ftrade_id is empty");
        return false;
    }
    
	string key = "user_latest_fund_trade_" + toString(trade_fund.Ftrade_id);

    vector<ST_TRADE_FUND> tradeFundVec;
	if( !QueryBatchTradeFund(mysql, trade_fund, tradeFundVec, false))
	{
		TRACE_DEBUG("no fund trade record");
		return true;
	}

    string szValue;
    packTradeRecordsCkvValue(tradeFundVec, szValue);
	
    //将szValue写入ckv
    //将老的key CKV_KEY_UID替换为新的key CKV_KEY_USER_LATEST_FUND_TRADE
	if(gCkvSvrOperator->set(CKV_KEY_USER_LATEST_FUND_TRADE, key, szValue,false))
	{
		return false;
	}
	else
	{
		return true;
	}
}

/**
* 检查授权限额 （use exau_check ）
* @input: pRpc exau_server连接句柄
* @input:iUid
* @input:vAmount
* @input:creId 证件号码
*/
void checkExauAuthLimit(CRpcWrapper* pRpc, int iUid, LONG vAmount, string creId,int redem_type)
{
    char    szMsg[MAX_MSG_LEN + 1];
    memset(szMsg, 0, sizeof(szMsg));

    int req_type = 38; // 设置金额校验参数  req_type = 38
    if (redem_type == DRAW_ARRIVE_TYPE_T1)
    {
        req_type = 40; 
    }
    
    CUrlAnalyze::setParam(szMsg, "req_type", req_type, true);
    CUrlAnalyze::setParam(szMsg, "channel_id", 1);
    CUrlAnalyze::setParam(szMsg, "direct", 1);
    CUrlAnalyze::setParam(szMsg, "uid", iUid);
    CUrlAnalyze::setParam(szMsg, "cre_id", creId.c_str());
    CUrlAnalyze::setParam(szMsg, "amount", toString(vAmount).c_str());
    CUrlAnalyze::setParam(szMsg, "redem_type", redem_type); // 预留

    try {
        return checkExauLimit(pRpc , szMsg,gPtrConfig->m_AppCfg.exau_sp_id.c_str()) ;
    } catch(CException &e) {
        /**
         * 87513001 超过单笔限额
         * 87513002 超过日限额
         * 87513003 超过日限次
         * 87513004 超过月限额
         * 87513005 超过月限次
         * exau服务对有OTP和无OTP的限额错误码相同，此处对无OTP的商家交易限额错误码进行转义
         * 有OTP的错误码保持exau返回的错误码值。
         */        
        
        throw CException(ERR_OVER_EXAU_LIMIT, e.what(), e.file(), e.line());
    }
}

void checkBalanceFetchExauAuthLimit(CRpcWrapper* pRpc, int iUid, LONG vAmount, string creId,int req_type)
{
    char    szMsg[MAX_MSG_LEN + 1];
    memset(szMsg, 0, sizeof(szMsg));
    
    CUrlAnalyze::setParam(szMsg, "req_type", req_type, true);
    CUrlAnalyze::setParam(szMsg, "channel_id", 1);
    CUrlAnalyze::setParam(szMsg, "direct", 1);
    CUrlAnalyze::setParam(szMsg, "uid", iUid);
    CUrlAnalyze::setParam(szMsg, "cre_id", creId.c_str());
    CUrlAnalyze::setParam(szMsg, "amount", toString(vAmount).c_str());

    try {
        return checkExauLimit(pRpc , szMsg,gPtrConfig->m_AppCfg.exau_sp_id.c_str()) ;
    } catch(CException &e) {
        /**
         * 87513001 超过单笔限额
         * 87513002 超过日限额
         * 87513003 超过日限次
         * 87513004 超过月限额
         * 87513005 超过月限次
         * exau服务对有OTP和无OTP的限额错误码相同，此处对无OTP的商家交易限额错误码进行转义
         * 有OTP的错误码保持exau返回的错误码值。
         */        
        
        throw CException(ERR_OVER_EXAU_LIMIT, e.what(), e.file(), e.line());
    }
}

/**
* 更新余额提现限额 （use exau_check ）
* @input:pRpc
* @input:iUid
* @input:vAmount
* @input:creId 证件号码
*/
void updateBalanceFetchExauAuthLimit(CRpcWrapper* pRpc, int iUid, LONG vAmount, string creId,int req_type)
{
    char    szMsg[MAX_MSG_LEN + 1];
    char    szBuf[MAX_MSG_LEN + 1];
    char    szResInfo[256] = {0};
    int    iResult = -1, oLen = sizeof(szBuf);

    memset(szMsg, 0x00, sizeof(szMsg));
    memset(szBuf, 0x00, sizeof(szBuf));
    memset(szResInfo, 0x00, sizeof(szResInfo));

    // 设置授权金额累加参数
    CUrlAnalyze::setParam(szMsg, "req_type", req_type, true);
    CUrlAnalyze::setParam(szMsg, "channel_id", 1);
    CUrlAnalyze::setParam(szMsg, "direct", 1);
    CUrlAnalyze::setParam(szMsg, "uid", iUid);
    CUrlAnalyze::setParam(szMsg, "cre_id", creId.c_str());
    CUrlAnalyze::setParam(szMsg, "amount", toString(vAmount).c_str());

    // 加密
    ST_PUB_ANS stAns; // 结果信息
    memset(&stAns, 0x00, sizeof(ST_PUB_ANS));
    char szEncode[MAX_MSG_LEN + 1] = {0};
    memset(szEncode, 0x00, sizeof(szEncode));

    encode(gPtrConfig->m_AppCfg.exau_sp_id.c_str(), szMsg, szEncode, stAns);

    if (stAns.iResult != 0)
    {
        throw CException(stAns.iResult, stAns.szErrInfo, __FILE__, __LINE__);
    }

    memset(szMsg, 0x00, sizeof(szMsg));
    snprintf(szMsg, sizeof(szMsg), "%s=%s&%s=%s", "sp_id", gPtrConfig->m_AppCfg.exau_sp_id.c_str(), "request_text", szEncode);

    // 调用 exau_limitsum_service 接口
    pRpc->excute("exau_limitsum_service", szMsg, strlen(szMsg), szBuf, oLen);
    
    // 获取结果
    CUrlAnalyze::getParam(szBuf, "result", &iResult);
    CUrlAnalyze::getParam(szBuf, "res_info", szResInfo, sizeof(szResInfo)-1);

    if( 0 != iResult )
    {
        throw CException(iResult, szResInfo, __FILE__, __LINE__);
    }

    return;
}

/**
*  检查限额服务
**/
void checkExauLimit(CRpcWrapper* pRpc, const char* pMsg , const char* pspid)
{

    char    szBuf[MAX_MSG_LEN + 1];
    char    szResInfo[256] = {0};
    int    iResult = -1, oLen = sizeof(szBuf);

    memset(szBuf, 0x00, sizeof(szBuf));
    memset(szResInfo, 0x00, sizeof(szResInfo));


    // 加密
    ST_PUB_ANS stAns; // 结果信息
    memset(&stAns, 0, sizeof(ST_PUB_ANS));

    char szEncode[MAX_MSG_LEN + 1] = {0};
    memset(szEncode, 0x00, sizeof(szEncode));

    encode(pspid , pMsg, szEncode, stAns);

    if (stAns.iResult != 0)
    {
        throw CException(stAns.iResult, stAns.szErrInfo, __FILE__, __LINE__);
    }

    char    szMsg[MAX_MSG_LEN + 1];
    memset(szMsg, 0, sizeof(szMsg));
    snprintf(szMsg, sizeof(szMsg), "%s=%s&%s=%s&MSG_NO=%s", "sp_id", pspid,
             "request_text", szEncode,MSG_NO);

    // 调用 exau_limitcheck_service 接口
    pRpc->excute("exau_limitcheck_service", szMsg, strlen(szMsg), szBuf, oLen);

    // 获取结果
    CUrlAnalyze::getParam(szBuf, "result", &iResult);
    CUrlAnalyze::getParam(szBuf, "res_info", szResInfo, sizeof(szResInfo)-1);

    if (0 != iResult)
    {
        throw CException(iResult, szResInfo, __FILE__, __LINE__);
    }

    return;
}


/**
* 更新赎回限额 （use exau_check ）
* @input:pRpc
* @input:iUid
* @input:vAmount
* @input:creId 证件号码
*/
void updateExauAuthLimit(CRpcWrapper* pRpc, int iUid, LONG vAmount, string creId,int redem_type)
{
    char    szMsg[MAX_MSG_LEN + 1];
    char    szBuf[MAX_MSG_LEN + 1];
    char    szResInfo[256] = {0};
    int    iResult = -1, oLen = sizeof(szBuf);

    memset(szMsg, 0x00, sizeof(szMsg));
    memset(szBuf, 0x00, sizeof(szBuf));
    memset(szResInfo, 0x00, sizeof(szResInfo));

    int req_type = 38; // 设置金额校验参数  req_type = 38
    if (redem_type == DRAW_ARRIVE_TYPE_T1)
    {
        req_type = 40; 
    }
    // 设置授权金额累加参数
    CUrlAnalyze::setParam(szMsg, "req_type", req_type, true);
    CUrlAnalyze::setParam(szMsg, "channel_id", 1);
    CUrlAnalyze::setParam(szMsg, "direct", 1);
    CUrlAnalyze::setParam(szMsg, "uid", iUid);
    CUrlAnalyze::setParam(szMsg, "cre_id", creId.c_str());
    CUrlAnalyze::setParam(szMsg, "amount", toString(vAmount).c_str());
    CUrlAnalyze::setParam(szMsg, "redem_type", redem_type); // 预留

    // 加密
    ST_PUB_ANS stAns; // 结果信息
    memset(&stAns, 0x00, sizeof(ST_PUB_ANS));
    char szEncode[MAX_MSG_LEN + 1] = {0};
    memset(szEncode, 0x00, sizeof(szEncode));

    encode(gPtrConfig->m_AppCfg.exau_sp_id.c_str(), szMsg, szEncode, stAns);

    if (stAns.iResult != 0)
    {
        throw CException(stAns.iResult, stAns.szErrInfo, __FILE__, __LINE__);
    }

    memset(szMsg, 0x00, sizeof(szMsg));
    snprintf(szMsg, sizeof(szMsg), "%s=%s&%s=%s", "sp_id", gPtrConfig->m_AppCfg.exau_sp_id.c_str(), "request_text", szEncode);

    // 调用 exau_limitsum_service 接口
    pRpc->excute("exau_limitsum_service", szMsg, strlen(szMsg), szBuf, oLen);
    
    // 获取结果
    CUrlAnalyze::getParam(szBuf, "result", &iResult);
    CUrlAnalyze::getParam(szBuf, "res_info", szResInfo, sizeof(szResInfo)-1);

    if( 0 != iResult )
    {
        throw CException(iResult, szResInfo, __FILE__, __LINE__);
    }

    return;
}

bool checkWhitePayUser(string uin)
{
	vector<string> payCardWhiteVec = gPtrConfig->m_AppCfg.payCardWhiteVec;
	vector<string>::iterator authTypeVecIt = payCardWhiteVec.begin();

	//实名认证方式在配置列表中的，认为已是实名认证用户
	for(unsigned int i =  0; i < payCardWhiteVec.size(); i++)
	{
		if(!authTypeVecIt[i].empty() && authTypeVecIt[i] == uin)
		{
			return true;
		}
	}

	return false;
}


//根据申购单号查询提现请求表(判断是否是提现失败重新申购)
int queryFundFetchList(CMySQL* mysql,const string &fundTransid,const string &date,const string &Fnum,string Fuin)
{
    char    szSqlStr[MAX_SQL_LEN*2+1]={0};
    MYSQL_RES*    pResult = NULL;

    // 转义单号
    string sEscapefundTransid = escapeString(fundTransid);

    int iLen;
		if(Fuin.empty())
		{
			iLen = snprintf(szSqlStr, sizeof(szSqlStr), 
				" SELECT Fstate,Fnum,Fuid  "
				" FROM fund_db.t_fetch_list_%s  "
				" WHERE Ffund_apply_id='%s' AND Flstate=1 AND Fstate=3  AND Fnum=%s ", 
					date.c_str(),sEscapefundTransid.c_str(),Fnum.c_str());
		}
		else
		{
			iLen = snprintf(szSqlStr, sizeof(szSqlStr), 
				" SELECT Fstate,Fnum,Fuid  "
				" FROM fund_db.t_fetch_list_%s  "
				" WHERE Ffund_apply_id='%s' AND Flstate=1 AND Fstate=3  AND Fnum=%s AND Fuin='%s' ", 
					date.c_str(),sEscapefundTransid.c_str(),Fnum.c_str(),Fuin.c_str());
		}

    mysql->Query(szSqlStr, iLen);

    pResult = mysql->FetchResult();
    if(0 >= mysql_num_rows(pResult))
    {
        mysql_free_result(pResult);
        return 0;
    }
    
    //MYSQL_ROW row = mysql_fetch_row(pResult);

    mysql_free_result(pResult);
    return 1;
}

//判断提现失败重新申购

bool checkInnerBalancePayForReBuy(CMySQL* mysql,const string &fundTransid,const string &Fnum,string Fuin)
{
    if (fundTransid.length()<18)//异常检查
    {
        return false;
    }
    
    //单号日期校验
    int date_yyyy = atoi(fundTransid.substr(10,4).c_str());
    int date_mm = atoi(fundTransid.substr(14,2).c_str());
    if (date_yyyy<2013 || date_mm>12 || date_mm<=0)
    {
        TRACE_ERROR("invalid fundTransid=%s",fundTransid.c_str());
        return false;
    }
    
    int rowNums = 0;
    rowNums = queryFundFetchList(mysql,fundTransid,fundTransid.substr(10,6),Fnum,Fuin);
    if (rowNums<=0) //当月没有查询到，往前查询一个月
    {
        string lastMonth = addDays(fundTransid.substr(10,6)+"01", -2).substr(0,6);
        rowNums = queryFundFetchList(mysql,fundTransid,lastMonth,Fnum,Fuin);
    }
    
    return (rowNums==1);
    

}

void setStopFetchFlag(CMySQL* pMysql)
{
    //将配置写入ckv
    string key = "t_fund_config_stop_fetch_flag";
    string value = "Fvalue=1";
    //louis临时的T+1 没有上线时搞的标记，现已不使用，key no置为负数表示无效
    gCkvSvrOperator->set(CKV_KEY_INVALID_KEYNO, key, value);
    
    FundConfig data;
    memset(&data,0,sizeof(data));
    strncpy(data.Fkeyword,"stop_fetch_flag",sizeof(data.Fkeyword)-1);
    if (true == queryFundConfig(pMysql, data,  false))
    {
        return;
    }
    memset(&data,0,sizeof(data));
    strncpy(data.Fkeyword,"stop_fetch_flag",sizeof(data.Fkeyword)-1);
    strncpy(data.Fvalue,"1",sizeof(data.Fvalue)-1);


    char szTimeNow[MAX_TIME_LENGTH+1] = {0};
    GetTimeNow(szTimeNow);
    strncpy(data.FcreateTime,szTimeNow,sizeof(data.FcreateTime)-1);
    strncpy(data.FmodifyTime,szTimeNow,sizeof(data.FmodifyTime)-1);
    insertFundConfig(pMysql, data);

}


/**
 * 新增基金账户注销记录
 */
void InsertFundUnBind(CMySQL* mysql, ST_FUND_UNBIND* pstRecord)
{
    char    szSqlStr[MAX_SQL_LEN*2+1] = {0};

    int iLen = snprintf(szSqlStr, sizeof(szSqlStr), 
        "replace into fund_db.t_fund_unbind "
        "(Fcre_type, Fcre_id, Ftrade_id, Fqqid, Fuid, Ftrue_name, Fcre_id_orig, Fphone, Fmobile, Fstate, Flstate, Fcreate_time, Fmodify_time,Facct_type,Fchannel_id,Fopenid,Facc_time ) values "
        "(%d, '%s', '%s', '%s', %d, '%s', '%s', '%s', '%s', %d, %d, '%s', '%s', %d, '%s', '%s', '%s')", 
        pstRecord->Fcre_type, escapeString(pstRecord->Fcre_id).c_str(), escapeString(pstRecord->Ftrade_id).c_str(), 
        escapeString(pstRecord->Fqqid).c_str(), pstRecord->Fuid, 
        escapeString(pstRecord->Ftrue_name).c_str(), escapeString(pstRecord->Fcre_id_orig).c_str(),
        escapeString(pstRecord->Fphone).c_str(), escapeString(pstRecord->Fmobile).c_str(),
        pstRecord->Fstate, pstRecord->Flstate, pstRecord->Fcreate_time, pstRecord->Fmodify_time,
        pstRecord->Facct_type, escapeString(pstRecord->Fchannel_id).c_str(), escapeString(pstRecord->Fopenid).c_str(),pstRecord->Facc_time);

    mysql->Query(szSqlStr, iLen);

    // 判断影响行数
    int affectedNums = mysql->AffectedRows();
    if(affectedNums<1||affectedNums>2)
    {
        gPtrAppLog->error("insert fundbind affected rows[%d] error, sql[%s]", mysql->AffectedRows(), szSqlStr); 
        throw CException(ERR_DB_AFFECT_MULTIROWS, "insert fundbind affected rows error! ", __FILE__, __LINE__);
    }
}

void recoveryFundBind(CMySQL* mysql, ST_FUND_UNBIND &unbindInfo)
{
    string tb_name;
    stringstream ss_cond;
    map<string, string> kv_map;
    
    tb_name = "fund_db.t_fund_bind";

    // 设置需要更新的字段
    kv_map["Fqqid"] = unbindInfo.Fqqid;
    kv_map["Fmodify_time"] = unbindInfo.Fmodify_time;
    kv_map["Flstate"] = "1";
    
    ss_cond << "Ftrade_id='" << escapeString(unbindInfo.Ftrade_id) << "'";
    
    // 执行更新数据表操作
    int affect_row = UpdateTable(mysql, tb_name, ss_cond, kv_map);
    if (affect_row != 1)
    {
        throw EXCEPTION(ERR_DB_AFFECT_MULTIROWS, "update affect rows error");
    }
}

void updateSignForFundBind(CMySQL* mysql, ST_FUND_BIND &bindInfo)
{
    string tb_name;
    stringstream ss_cond;
    map<string, string> kv_map;
    
    tb_name = "fund_db.t_fund_bind";

    // 设置需要更新的字段
    kv_map["Fstandby11"] = genSign("t_fund_bind", bindInfo);    
    ss_cond << "Ftrade_id='" << escapeString(bindInfo.Ftrade_id) << "'";
    
    // 执行更新数据表操作
    int affect_row = UpdateTable(mysql, tb_name, ss_cond, kv_map);
    if (affect_row != 1)
    {
        gPtrAppLog->error("update fund_db.t_fund_bind affect rows error,trade_id=%s", bindInfo.Ftrade_id);
    }
}

/**
 * 查询用户是否注销
 */
bool checkIsUserUnbind(CMySQL* mysql, int uid)
{
    char    szSqlStr[MAX_SQL_LEN*2+1] = {0};
    MYSQL_RES* pResult = NULL;
    int iLen = snprintf(szSqlStr, sizeof(szSqlStr), 
        "select Fuid  "
        " from fund_db.t_fund_unbind "
        " where Fuid=%d and  Flstate=1 ", 
        uid);

    mysql->Query(szSqlStr, iLen);

    pResult = mysql->FetchResult();

    int row_num = mysql_num_rows(pResult);
    mysql_free_result(pResult);
    
    return (row_num>0?true:false);
}

/**
 * 查询用户最近一笔注销记录
 */
int getLastUnbindUid(CMySQL* mysql, ST_FUND_UNBIND &unbindInfo)
{
    char    szSqlStr[MAX_SQL_LEN*2+1] = {0};
    MYSQL_RES* pResult = NULL;
    int uid=0;
    try
    {
        int iLen = snprintf(szSqlStr, sizeof(szSqlStr), 
            "select Fuid,Ftrade_id,Fqqid  "
            " from fund_db.t_fund_unbind "
            " where Fqqid='%s' and  Flstate=1  order by Facc_time DESC limit 1 ", 
            unbindInfo.Fqqid);

        mysql->Query(szSqlStr, iLen);

        pResult = mysql->FetchResult();

        // 获取结果行
        int iRow = mysql_num_rows(pResult);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
            
        MYSQL_ROW row = mysql_fetch_row(pResult);  
        
        uid = (row[0] ? atoi(row[0]) : 0);
        unbindInfo.Fuid = uid;
        strncpy(unbindInfo.Ftrade_id, ValiStr(row[1]), sizeof(unbindInfo.Ftrade_id)-1);
        strncpy(unbindInfo.Fqqid, ValiStr(row[2]), sizeof(unbindInfo.Fqqid)-1);
        mysql_free_result(pResult);
    }
    catch(CException& e)
    {
        if(pResult)    mysql_free_result(pResult);
        throw;
    }
    catch( ... )
    {
        if(pResult)    mysql_free_result(pResult);
        throw;
    }
    return uid;
}

/**
 * 查询销户表信息
 */
bool getUnbindInfoByTradeid(CMySQL* mysql, ST_FUND_UNBIND &unbindInfo)
{
    char    szSqlStr[MAX_SQL_LEN*2+1] = {0};
    MYSQL_RES* pResult = NULL;
    try
    {
        int iLen = snprintf(szSqlStr, sizeof(szSqlStr), 
            "select Fuid,Fqqid  "
            " from fund_db.t_fund_unbind "
            " where Ftrade_id='%s'  ", 
            unbindInfo.Ftrade_id);

        mysql->Query(szSqlStr, iLen);

        pResult = mysql->FetchResult();

        // 获取结果行
        int iRow = mysql_num_rows(pResult);
        if(iRow != 1)
        {
            mysql_free_result(pResult);
            return false;
        }
            
        MYSQL_ROW row = mysql_fetch_row(pResult);  
        
        unbindInfo.Fuid = (row[0] ? atoi(row[0]) : 0);
        strncpy(unbindInfo.Fqqid, ValiStr(row[1]), sizeof(unbindInfo.Fqqid)-1);
        mysql_free_result(pResult);
    }
    catch(CException& e)
    {
        if(pResult)    mysql_free_result(pResult);
        throw;
    }
    catch( ... )
    {
        if(pResult)    mysql_free_result(pResult);
        throw;
    }
    return true;
}

/**
 * 基金账户注销更新绑定记录为无效，并更新qqid为tradeid
 */
void disableFundBind(CMySQL* mysql, ST_FUND_BIND* pstRecord)
{
    string tb_name;
    stringstream ss_cond;
    map<string, string> kv_map;
    
    tb_name = "fund_db.t_fund_bind";

    // 设置需要更新的字段
    ST_FUND_BIND stFundBind;    
    strncpy(stFundBind.Fqqid, pstRecord->Fqqid, sizeof(stFundBind.Fqqid) - 1);
    strncpy(stFundBind.Ftrade_id, pstRecord->Ftrade_id, sizeof(stFundBind.Ftrade_id) - 1);
    strncpy(stFundBind.Fcre_id,pstRecord->Fcre_id, sizeof(stFundBind.Fcre_id) - 1);
    stFundBind.Fuid = pstRecord->Fuid;
    strncpy(stFundBind.Fmobile,pstRecord->Fmobile, sizeof(stFundBind.Fmobile) - 1);
    strncpy(stFundBind.Fopenid,pstRecord->Fopenid, sizeof(stFundBind.Fopenid) - 1);            
    kv_map["Fqqid"] = pstRecord->Ftrade_id;
    kv_map["Fmodify_time"] = pstRecord->Fmodify_time;
    kv_map["Fstandby11"] = genSign("t_fund_bind", stFundBind);
    kv_map["Flstate"] = "2";
    
    ss_cond << "Ftrade_id='" << escapeString(pstRecord->Ftrade_id) << "'";
    
    // 执行更新数据表操作
    int affect_row = UpdateTable(mysql, tb_name, ss_cond, kv_map);
    if (affect_row != 1)
    {
        throw EXCEPTION(ERR_DB_AFFECT_MULTIROWS, "update affect rows error");
    }
}

/**
 * 注销后重新恢复账户把注销表记录改为无效
 */
void disableFundUnBind(CMySQL* mysql, ST_FUND_UNBIND &unbindRecord)
{
    string tb_name;
    stringstream ss_cond;
    map<string, string> kv_map;
    
    tb_name = "fund_db.t_fund_unbind";

    // 设置需要更新的字段
    kv_map["Fmodify_time"] = unbindRecord.Fmodify_time;
    kv_map["Flstate"] = "2";
    
    ss_cond << "Ftrade_id='" << escapeString(unbindRecord.Ftrade_id) << "'";
    
    // 执行更新数据表操作
    int affect_row = UpdateTable(mysql, tb_name, ss_cond, kv_map);
    if (affect_row != 1)
    {
        throw EXCEPTION(ERR_DB_AFFECT_MULTIROWS, "update affect rows error");
    }
}


/**
 * 统计用户交易笔数
 */
int countTranRecords(CMySQL* mysql,int uid,const string &cond)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    int count=0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " count(Fuid) "
                    " FROM fund_db_%02d.t_trade_user_fund_%d "
                    " WHERE "
                    " Fuid=%d  " 
                    " %s ",
                    Sdb1(uid),
                    Stb1(uid),
                    uid,
                    cond.c_str()
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        mysql->Query(szSql, iLen);
        // 取结果集
        pRes = mysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        
        MYSQL_ROW row = mysql_fetch_row(pRes);  
        count = row[0] ? atoll(row[0]) : 0;

        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return count;
}

/**
 * 统计用户申购交易金额
 */
LONG getBuyRecordsFee(CMySQL* mysql,int uid,const string &cond)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    LONG total_fee=0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " sum(Ftotal_fee)  "
                    " FROM fund_db_%02d.t_trade_user_fund_%d "
                    " WHERE "
                    " Fuid=%d  " 
                    " %s ",
                    Sdb1(uid),
                    Stb1(uid),
                    uid,
                    cond.c_str()
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        mysql->Query(szSql, iLen);
        // 取结果集
        pRes = mysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        
        MYSQL_ROW row = mysql_fetch_row(pRes);  
        total_fee = row[0] ? atoll(row[0]) : 0;

        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return total_fee;
}

/**
 * 统计用户赎回交易金额
 */
LONG getRedemRecordsFee(CMySQL* mysql,int uid,const string &cond)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    LONG total_fee=0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " sum(IF(Freal_redem_amt>0,Freal_redem_amt,Ftotal_fee))  "
                    " FROM fund_db_%02d.t_trade_user_fund_%d "
                    " WHERE "
                    " Fuid=%d  " 
                    " %s ",
                    Sdb1(uid),
                    Stb1(uid),
                    uid,
                    cond.c_str()
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        mysql->Query(szSql, iLen);
        // 取结果集
        pRes = mysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        
        MYSQL_ROW row = mysql_fetch_row(pRes);  
        total_fee = row[0] ? atoll(row[0]) : 0;

        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return total_fee;
}

/**
 * 统计用户交易笔数
 */
int countFetchingRecords(CMySQL* mysql,const string& uin,const string &yyyymm)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    int count=0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " count(Fuin) "
                    " FROM fund_db.t_fetch_list_%s "
                    " WHERE "
                    " Fuin='%s'  AND Flstate=1 AND (Fstate=3 OR Fstate=2) ",
                    escapeString(yyyymm).c_str(),
                    escapeString(uin).c_str()
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        mysql->Query(szSql, iLen);
        // 取结果集
        pRes = mysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        
        MYSQL_ROW row = mysql_fetch_row(pRes);  
        count = row[0] ? atoll(row[0]) : 0;

        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return count;
}

/**
 * 统计用户提现中的金额
 */
LONG getFetchingRecordsFee(CMySQL* mysql,const string& uin,const string &yyyymm)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    LONG total_fee=0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " sum(Fnum) "
                    " FROM fund_db.t_fetch_list_%s "
                    " WHERE "
                    " Fuin='%s'  AND Flstate=1 AND (Fstate=3 OR Fstate=2) ",
                    escapeString(yyyymm).c_str(),
                    escapeString(uin).c_str()
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        mysql->Query(szSql, iLen);
        // 取结果集
        pRes = mysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        
        MYSQL_ROW row = mysql_fetch_row(pRes);  
        total_fee = row[0] ? atoll(row[0]) : 0;

        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return total_fee;
}

 /*
 *查询成功ret >0 结果保存到result
 */
int doBatchSelect(CMySQL* mysql, const string&sql,CStr2sVec &result)
{
    MYSQL_RES* pRes =NULL;
    unsigned int  iRow = 0;
    try
    {
        // 执行查询
        mysql->Query(sql.c_str(), sql.length());

        // 取结果集
        pRes = mysql->FetchResult();

        // 获取结果行
        iRow = mysql_num_rows(pRes);

        if (iRow == 0)
        {
            if(pRes)    
            {
                mysql_free_result(pRes);
            }
            return 0;
        }

        unsigned int fieldNum = mysql_num_fields(pRes);
        
        MYSQL_FIELD *fields = mysql_fetch_fields(pRes);

        for (unsigned int rowIdx=0;rowIdx<iRow;rowIdx++)
        {

            MYSQL_ROW row = mysql_fetch_row(pRes);

            for (unsigned int i=0; i<fieldNum; i++)
            {
                result[(fields[i].name)].push_back(ValiStr(row[i]));
            }

        }
    }
    catch(CException& e)
    {
        if(pRes)    
        {
            mysql_free_result(pRes);
        }
        throw;
    }
    if(pRes)    
    {
        mysql_free_result(pRes);
    }
    return iRow;
}

 /*
 *查询商户保有量
 */
LONG QuerySpTotalBalance(CMySQL* mysql, const string &spid)
{
    stringstream ss_sql;
    MYSQL_RES* result = NULL;
    MYSQL_ROW  row;

    ss_sql << "select Fbalance ";
    ss_sql << " from fund_db.t_fund_recon_stat  where  Fspid='"<<escapeString(spid)<<"' and Fbalance>0 order by Frecon_date DESC limit 1 ";

    mysql->Query(ss_sql.str().c_str(), ss_sql.str().size());

    result = mysql->FetchResult();
    int rownum = mysql_num_rows(result);
    if (0 == rownum)
    {
        mysql_free_result(result);
        return 0;
    }
    if (rownum > 1)
    {
        mysql_free_result(result);
        throw CException(ERR_UNIQUE_MULTIROWS, "query t_fund_recon_stat get multiple rows! ", __FILE__, __LINE__);
    }

    row = mysql_fetch_row(result);

    LONG totalfee = atoll(ValiStr(row[0]));
   
    mysql_free_result(result);

    TRACE_DEBUG("query t_fund_recon_stat exist,totalfee=%zd",totalfee);
    return totalfee;
}

 void checkSpRedemRateLimit(CMySQL* mysql, FundSpConfig &spconfig,const string &systime,LONG transferFee)
 {
    if(!queryFundSpAndFundcodeConfig(mysql, spconfig, false))
    {
    	//不应该发生
    	throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
    }
    
    LONG spTotalBalance = QuerySpTotalBalance(mysql,spconfig.Fspid);
    LONG cur_TotalRedemFee = 0;
    
    if (spconfig.Fredem_day == getYYYYMMDDFromStdTime(systime))
    {
        cur_TotalRedemFee = spconfig.Fredem_total_day;
        //LONG spTotalBalance = QuerySpTotalBalance(m_pFundCon,addDays(m_fund_orisp_config.Fredem_day,-1), m_params["ori_spid"]);
    }
    
    int rate = (gPtrConfig->m_spTransferRate[spconfig.Fspid]>0?gPtrConfig->m_spTransferRate[spconfig.Fspid]:gPtrConfig->m_AppCfg.sp_redem_rate);
    
    if (spTotalBalance<(cur_TotalRedemFee+transferFee)*rate) 
    {
        gPtrAppLog->error("sp transfer overceed limit one day,spid=%s,spTotalBalance=%zd, total_fee=%zd", 
			                 spconfig.Fspid, spTotalBalance,transferFee);
        throw CException(ERR_CANNOT_CHANGESP_AGAIN, "sp transfer overceed limit one day", __FILE__, __LINE__);
    }
 }

//10位商户号+ 8位日期+ 10位随机数
string GenerateIdsBySpid(const string &spid)
{
    char  listid[MAX_LISTID_LENGTH+1] = {0};

    // 取billno
    unsigned int billno = getBillno(gPtrConfig->m_BillnoCfg.host, gPtrConfig->m_BillnoCfg.port, gPtrConfig->m_BillnoCfg.appid);

    // 取当前时间
    time_t tt = time(NULL);
    struct tm tm1;
    localtime_r(&tt, &tm1);

    int date = (tm1.tm_year + 1900) * 10000 + (tm1.tm_mon + 1) * 100 + tm1.tm_mday;
    snprintf(listid, sizeof(listid),  "%s%d%.10u", spid.c_str(),date, billno);
    return listid;
}

bool checkTransIdAndSpid(const string &spid,const string &transid)
{
    if (transid.length()<28)
    {
        return false; 
    }
    if (spid.length()<1)
    {
        return false; 
    }
    if (transid.find(spid) != 0)
    {
        return false;
    }
    return true;
}

//判断用户总资产是否超出限制
bool isUserAssetOverLimit(int limitLevel,LONG currentTotalAsset,LONG totalFee)
{
    LONG Limit = gPtrConfig->m_AppCfg.user_max_share;
    if (gPtrConfig->m_assetLimitLev2Value.find(limitLevel) != gPtrConfig->m_assetLimitLev2Value.end())
    {
        Limit = gPtrConfig->m_assetLimitLev2Value[limitLevel];
    }
    else
    {
        //等级未配置告警提示
        alert(ERR_BAD_PARAM, string("asset level:")+toString(limitLevel)+string("not configed!! set to default"));
    }

    TRACE_DEBUG("user limitLevel=%d,LimitValue=%ld,currentTotalAsset=%ld,totalFee=%ld",limitLevel,Limit,currentTotalAsset,totalFee);

    if (currentTotalAsset+totalFee > Limit)
    {
        return true;
    }
    return false;
}

/**
 * 获取用户风险评级结果 
 */
int getAgreeRiskType(FundSpConfig& spConfig,ST_FUND_BIND& fundBind)
{	
	int userRiskType = fundBind.Fassess_risk_type;
	if(spConfig.Frisk_ass_flag == SPCONFIG_RISK_NONE || userRiskType >= spConfig.Frisk_type )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
bool checkUserWhiteList(const string &key, const string &uid)
{
    // 根据key从CKV中找到对应的白名单
	string strKey = key;
    string value;
    int ret = gCkvSvrOperator->get(strKey, value);
    if(ret != 0)
    {
        return false;
    }
    else
    {
        CStr2Map outputMap;
        Tools::StrToMap(outputMap, value);
        string strType = outputMap["type"];		// 1-标识内部名单，2-标识尾号灰度
		string strList = outputMap["list"];
		if (strType == "2" && strList == "ALL")		// 标识全量放开
			return true;

		vector<string> vecWhiteList;
		str_split(strList, '|', vecWhiteList,false);
		for (vector<string>::size_type i=0; i<vecWhiteList.size(); ++i)
		{
            //如果分隔的项为空则忽略
            if (vecWhiteList[i].empty())
                continue;
            
			if (strType == "1" && vecWhiteList[i] == uid)
				return true;
			if (strType == "2" && (vecWhiteList[i].size() <= uid.size() && vecWhiteList[i] == uid.substr(uid.size() - vecWhiteList[i].size())))
				return true;
		}
    }
    return false;
}

//key 可以直接传uin，也可以传带前缀的真实key
string getUserWhiteListValue(string key)
{
    if (key.find("pc_charge_white_list_") !=0)
    {
        key=string("pc_charge_white_list_")+key;
    }

    CParams rawValue;
    int ret = gCkvSvrOperator->get(key, rawValue);
    if (ret != 0 && ret != -13200)
    {
        throw CException(ERR_GET_CKV_FAILED, "getUserWhiteListValue from ckv fail!  ", __FILE__, __LINE__);
    }

    return rawValue["Fpc_charge"];    
}

bool updateUserWhiteListValue(string key,const string &value)
{
    if (key.find("pc_charge_white_list_") !=0)
    {
        key=string("pc_charge_white_list_")+key;
    }

    CParams NVpara;
    NVpara.readIntParam(value.c_str(), "byte_index",0,256);
    NVpara.readIntParam(value.c_str(), "byte_value",0,9);

    string whiteListValue=getUserWhiteListValue(key);
    int len = whiteListValue.length();
    
    if (len<=NVpara.getInt("byte_index"))
    {
        whiteListValue.append(NVpara.getInt("byte_index")+1-len,'0');
    }

    whiteListValue[NVpara.getInt("byte_index")]='0'+NVpara.getInt("byte_value");
    
    string newValue = string("Fpc_charge=")+whiteListValue;
    int ret = gCkvSvrOperator->set(NOT_REUPDATE_CKV(CKV_KEY_PC_CHARGE_WHITELIST), key, newValue);

    return (ret==0);
    
}

// 把string乘以10的power次方取整转换为LONG型
// 只支持15个字符数以下的正数转换
// 失败返回-1;成功返回0
int str2Long(const string &sFloat,LONG &iFee,const int power )
{
	if(sFloat.size()>15)
	{
		return -1;
	}
	LONG iInteger;
	char szDecimal[20]={0};
	int iCount	= sscanf (sFloat.c_str(),"%zd.%[0-9]",&iInteger,szDecimal);
	if( iCount != 1 && iCount != 2 )
	{
		return -1;
	}
	szDecimal[power+1]='\0';
	LONG iDecimal=0;
	bool end = false;
	for(int i=0;i<power;i++)
	{
		iDecimal *= 10;
		if(end)
		{
			continue;
		}
		if('0'<=szDecimal[i] && '9'>=szDecimal[i])
		{
			iDecimal += szDecimal[i] - '0';
		}
		else
		{
			end = true;
		}
	}
	if(szDecimal[power]>='5'&&szDecimal[power]<='9' && end ==false)
	{
		iDecimal++;
	}
	for(int i=0;i<power;i++){
		iInteger = iInteger*10;
	}
	iFee = iInteger + iDecimal;
	return 0;
}

/**隐藏证件号码
 */
void hideCreId(const char* creId, int cretype,char* creHideId)
{
	if(creId==NULL)
	{
		creHideId[0]='\0';
		return;
	}
	// 身份证:从第2位开始隐藏,隐藏16位,只显示前1位和后1位
	if(cretype==1) 
	{
		hideChar(creId,creHideId,1,16);
		return;
	}
	// 其他证件大于4位:显示第一位标志符,数字只显示前1位和后1位
	// 小于等于4位:显示第一位标志符,数字全部打码
	int len=strlen(creId);
	if(len<=0)
	{
		creHideId[0]='\0';
		return;
	}
	int startHide=2; // 默认第一位为字母标志位,显示第二位
	if(creId[0]>='0'&&creId[0]<='9') // 第一位为数字则起始位从1开始
	{
		startHide=1;
	}
	
	if(len>4)
	{
		hideChar(creId,creHideId,startHide,len-1-startHide);
	}else{
		hideChar(creId,creHideId,startHide-1,len-startHide+1);
	}
}

/*
*  部分隐藏用户姓名
*  @para:   name---gbk编码用户姓名
*  只隐藏最后一个字
*/
void hideName(const char* name, char* nameHide)
{
	if(name==NULL)
	{
		nameHide[0]='\0';
		return;
	}
	unsigned int nameLen=strlen(name);

	unsigned int nameIdx=0;
	unsigned int hideIdx=0;
	
	// 大于两个中文长度
	if(nameLen>4) 
	{   
		// 第一个是中文,取前两字符
		if((unsigned int)name[0] >= 0x81 && (unsigned int)name[1] >=0x40)
		{
			nameHide[hideIdx++] = name[nameIdx++];
			nameHide[hideIdx++] = name[nameIdx++];
		}else{
			nameHide[hideIdx++] = name[nameIdx++];
		}
	}
	for(;nameIdx<nameLen;nameIdx++)
	{
		// 隐藏最后一位英文字符
		if(nameIdx>=nameLen-1)
		{
			nameHide[hideIdx++] = '*';
			break;
		}
		
		bool isCharacter = false;
		
		// 判断中文
		if((unsigned int)name[nameIdx] >= 0x81 && (unsigned int)name[nameIdx+1] >=0x40)
		{
			isCharacter=true;
		}
		// 隐藏2字符中文
		if(nameIdx>=nameLen-2&&isCharacter)
		{
			nameHide[hideIdx++] = '*';
			nameIdx++;
		// 显示中文
  		}else if(isCharacter)
		{
			nameHide[hideIdx++] = name[nameIdx++];
			nameHide[hideIdx++] = name[nameIdx];
		// 显示英文
		}else{
			nameHide[hideIdx++] = name[nameIdx];
		}
	}
	nameHide[hideIdx]='\0';
}

/*
*  部分隐藏字符
*/
void hideChar(const char* str, char* hide,unsigned int hideStart, unsigned int hideLenth)
{
	if(str==NULL)
	{
		hide[0]='\0';
		return;
	}
	unsigned int len=strlen(str);

	unsigned int hideIdx=0;
	unsigned int hideCount=0;
	
	for(unsigned int strIdx=0;strIdx<len;strIdx++)
	{
		if(strIdx<hideStart)
		{
			hide[hideIdx++]=str[strIdx];
		}else if(hideCount<hideLenth)
		{
			hide[hideIdx++]='*';
			hideCount++;
		}else{
			hide[hideIdx++] = str[strIdx];
		}
	}
	// 隐藏星星不够则补充
	while(hideCount<hideLenth)
	{
		hide[hideIdx++]='*';
		hideCount++;
	}
	hide[hideIdx]='\0';
}
bool isUserExistRedemingRecords(CMySQL* pMysql,int uid)
{
    char    szSqlStr[MAX_SQL_LEN*2+1] = {0};
    MYSQL_RES* pResult = NULL;
    int iLen = snprintf(szSqlStr, sizeof(szSqlStr), 
        "SELECT Flistid "
                   " FROM  fund_db_%02d.t_trade_user_fund_%d "
                   " WHERE Fuid=%d AND Flstate=1  AND Fstate in (4,5,13)  limit 1", 
        Sdb1(uid), Stb1(uid), uid);
    pMysql->Query(szSqlStr, iLen);

    pResult = pMysql->FetchResult();

    int row_num = mysql_num_rows(pResult);
    mysql_free_result(pResult);
    
    return (row_num>0?true:false);
}

bool isExistsUnconfirmRedemtion(CMySQL* pMysql,int uid, const string& spid)
{
    char    szSqlStr[MAX_SQL_LEN*2+1] = {0};
    MYSQL_RES* pResult = NULL;
    int iLen = snprintf(szSqlStr, sizeof(szSqlStr), 
        "SELECT Flistid "
                   " FROM  fund_db_%02d.t_trade_user_fund_%d "
                   " WHERE Fuid=%d AND Flstate=1 AND Fstate in (4,13) and Fspid='%s' limit 1", 
        Sdb1(uid), Stb1(uid), uid,spid.c_str());
    pMysql->Query(szSqlStr, iLen);

    pResult = pMysql->FetchResult();

    int row_num = mysql_num_rows(pResult);
    mysql_free_result(pResult);
    
    return (row_num>0?true:false);
}
/**
 *  从身份证中获取性别
 *  0: 未知; 1: 男;2: 女
 *  根据身份证倒数第二位判断,奇数是男,偶数是女
 */
int getSexFromCreId(const char* creId,int creType)
{
	// 非标准身份证都返回0
	if(creId==NULL||creType!=1)
	{
		return 0;
	}
	if(strlen(creId)!=18)
	{
		return 0;
	}
	// 获取倒数第二位数字
	int sex = creId[16]-48;
	// 转换性别标识
	return 2-sex%2;
}

void CheckTradeLimit(CMySQL* mysql, string sDay) throw (CException)
{
	string sData, sHKData,standby1,standby3;
	queryFundTDateforCache(mysql, sDay, sData, sHKData);

	if (strncmp(sData.c_str(), sHKData.c_str(), sHKData.length()) != 0)
	{
		throw CException(ERR_HK_TRADE_DAY, "不是沪港通交易日", __FILE__, __LINE__);
	}
	
	/*if(toInt(standby1.c_str()) == ETF_TYPE_PART_NOT_TRADE_TIME && standby3.length() > 0 
		&& standby3.length() < 17)
	{
		throw CException(ERR_HK_TRADE_FORMAT,"沪港通交易时间格式不合法", __FILE__, __LINE__);
	}

	int hour1=0, minute1=0, second1=0, hour2=0, minute2=0, second2=0;
	sscanf(standby3.c_str(), "%02d:%02d:%02d-%02d:%02d:%02d", &hour1, &minute1, &second1,
		&hour2, &minute2, &second2);
			
	int  iBegin = 3600*hour1 + 60*minute1 + second1;
	int  iEnd = 3600*hour2 + 60*minute2 + second2;
	int  iNow = time(NULL)%86400;

	if(iNow >= iBegin && iNow <= iEnd)
	{
		throw CException(ERR_HK_TRADE_TIME, "不是沪港通交易时间段", __FILE__, __LINE__);
	}*/
}


