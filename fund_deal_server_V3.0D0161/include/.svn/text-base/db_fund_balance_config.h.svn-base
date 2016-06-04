#ifndef _DB_FUND_BALANCE_CONFIG_H_
#define _DB_FUND_BALANCE_CONFIG_H_

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

typedef struct
{
    int Fid;
    char Fbalance_spid[16+1];  //余额总商户号
    char Fcharge_spid[16+1];
    char Ffetch_backer_spid[16+1]; //余额提现垫资商户号
    char Fbalance_spid_qqid[64+1];
    char Ffetch_backer_qqid[64+1];   //余额提现垫资商户C
    char Ffund_fetch_spid[16+1];     //提现总商户号
    char Ffund_fetch_spid_qqid[64+1];
    char Ftrans_date[20+1];
    int Flstate;
    LONG Ftotal_redem_balance_old;   //昨日总赎回额度(转出)
    LONG Ftotal_redem_balance; // 今日总赎回额度(转出)
    LONG Ftotal_buy_balance_old; //昨日总余额申购额
    LONG Ftotal_buy_balance;      //今日总余额申购额
    LONG Ftotal_available_balance; //总账户 实际可用余额
    LONG Ffetch_backer_total_fee; //总垫资额
    LONG Ffetch_limit_offset;
    LONG Ftotal_baker_fee;           //t 日总提现垫资额
    LONG Ftotal_baker_fee_old;    //昨日总提现垫资额
    LONG Ftotal_t1fetch_fee;       //总t+1提现金额
    LONG Ftotal_t1fetch_fee_old; //昨日总t+1提现金额
    int Fflag;//bit 1 是否支持垫资，bit2 是否停止t+0提现
    char Fsign[64+1];
    char Fcreate_time[20+1];
    char Fmodify_time[20+1];
    char Fmemo[128+1];
}FundBalanceConfig;


/**
 * 理财通余额回补流水表
*/
typedef struct
{
    char    Flistid[32+1];     // 单号
    LONG    Ftotal_transfer_sup_fee;        // 回补金额
    LONG    Ftotal_buy_fee;    // T日总余额申购金额
    LONG    Ftotal_redem_fee;   // T日总余额赎回金额
    LONG    Ftotal_sup_backer_fee; //提现垫资回补金额
    char    Fpurchaser_id[64+1];     
    char    Fbargainor_id[64+1];     
    int Fcur_type;
    int     Ftype;    //1：转入余额 2：余额转出 3：提现回补
    int     Fstate;            // 0：初始 1:转换回补成功2:转换和提现都回补成功
    int     Flstate;           // 物理状态  1 - 有效； 2 - 无效
    char    Fcreate_time[20+1];  // 记录创建时间
    char    Fmodify_time[20+1];  // 记录最新修改时间
    char    Facc_time[20+1];	//
    char    Ftrans_date[20+1];	//
    char    Fsign[32+1];  
    char	Fmemo[128+1];
    char   Fbacker_qqid[64+1];     //提现垫资账户
    char   Fsup_backer_translist[32+1];     // 提现回补单号
    char    Fsup_backer_time[20+1];  // 提现垫资回补时间
} ST_BALANCE_RECOVER; 


#define DEFAULT_BALNCE_CONFIG_FID 1

#define  FETCH_LOAN_OVER 0x01   //  1 垫资耗尽

void recordTotalBalanceAccRoll(CMySQL* pMysql, const string &listid,int listType,LONG totalFee,const string&acctime,LONG totalAccBalance,int typeAdd=0);
bool queryFundBalanceRecoverRoll(CMySQL* pMysql, ST_BALANCE_RECOVER & data,  bool lock);
void insertFundFundBalanceRecoverRoll(CMySQL* pMysql, const ST_BALANCE_RECOVER & data);
void updateFundFundBalanceRecoverRoll(CMySQL* pMysql, const ST_BALANCE_RECOVER & data);

bool isFundBalanceRecoverDelayed(CMySQL* pMysql,const string& sysTime);

bool queryFundBalanceConfig(CMySQL* pMysql, FundBalanceConfig & data,  bool lock);

bool setFundBalanceConfigToCkv(CMySQL* pMysql);

bool setFundBalanceConfigToCkv(CMySQL* pMysql,  FundBalanceConfig & data,bool NeedQuery=false);

void updateFundbalanceConfigTradeDate(CMySQL*pMysql,const string &trade_date,const string &systime);

LONG updateFundBalanceConfigForRecover(CMySQL*pMysql,const FundBalanceConfig & data,LONG total_buy,LONG total_redem,LONG total_fee);

void updateFundBalanceConfigForRecoverFetchBacker(CMySQL*pMysql,const FundBalanceConfig & data,LONG total_fee);

void updateFundBalanceConfigForBuyRefund(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date);

void updateFundBalanceConfigForBuy(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date);

void updateFundBalanceConfigForFetch(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date,bool &bNeedBacker);

void updateFundBalanceConfigForT1FetchReq(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date);
void updateFundBalanceConfigForT1FetchCnf(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date);

void updateFundBalanceConfigForRedem(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date);

void updateFundBalanceConfigForCharge(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date);

void AddQuerySqlForswitchTransDate(string &querySql,const string &trade_date,LONG totalFee,int op_type);

BalanceConfigCache getCacheBalanceConfig(CMySQL* pMysql);

#endif

