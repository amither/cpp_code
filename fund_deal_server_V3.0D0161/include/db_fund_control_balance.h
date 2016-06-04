#ifndef _DB_FUND_CONTROL_BALANCE_H_
#define _DB_FUND_CONTROL_BALANCE_H_

#include "parameter.h"
#include "common.h"
#include "cftlog.h"
#include "sqlapi.h"
#include "trpcwrapper.h"
#include "runinfo.h"
#include <sstream>
#include  "tinystr.h"
#include  "tinyxml.h"
#include "decode.h"

/**
 * 用户理财通余额受限表
*/
typedef struct
{
    char    Fspid[15+1];       // 商户号
    char    Ffund_spid[15+1]; //基金商户号
    char    Ftrade_id[32+1];   // 基金交易账户对应ID
    LONG    Ftotal_fee;        // 总受限金额（包括累计收益）
    LONG Ftotal_profit;      //累计收益
    LONG Flast_profit;       //昨日收益
    char  Flast_profit_day[20+1];  //昨日收益日期
    char    Ffirst_profit_day[20+1];//首次收益日期
    char    Fuin[64+1];     
    int     Ftype;
    int     Fcur_type;            // 币种
    int     Flstate;           // 物理状态  1 - 有效； 2 - 无效
    char    Fcreate_time[20+1];  // 记录创建时间
    char    Fmodify_time[20+1];  // 记录最新修改时间
    char    Fcard_no[32+1];  // 预付卡号
    char    Fcard_partner[32+1];  // 预付卡门店商户号
} ST_FUND_CONTROL_INFO; 

bool isConsumFundSpid(const string&fundSpid);

bool isWxPrePayCardBusinessUser(CMySQL* pMysql,const string&fundSpid,const string& trade_id);

bool isWxPrePayCardBusinessUser(CMySQL* pMysql,const string&fundSpid,ST_FUND_CONTROL_INFO &controlInfo);

bool checkWxPreCardBuy(CMySQL* pMysql, ST_FUND_CONTROL_INFO& controlParams, ST_FUND_CONTROL_INFO& controlInfo,bool lock);

bool queryFundControlInfo(CMySQL* pMysql, ST_FUND_CONTROL_INFO& data,  bool lock);

void insertFundControlInfo(CMySQL* pMysql, ST_FUND_CONTROL_INFO& data );

void addFundControlBalance(CMySQL* pMysql, LONG addFee,const string&systime,const string& trade_id,int business_type=1);
void subFundControlBalance(CMySQL* pMysql, LONG subFee ,const string&systime,const string& trade_id,int business_type=1);
void addFundControlProfit(CMySQL* pMysql, LONG addProfitFee,const string &profitDay,const string&systime,const string& trade_id,int business_type=1);

   
#endif