#ifndef _DB_FUND_PROFIT_H_
#define _DB_FUND_PROFIT_H_

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
#include "globalconfig.h"
#include "fund_commfunc.h"

//用户总收益ckv的超时时间
#define DEF_USR_TOTAL_PROFIT_CKV_TIMEOUT (60*60*16)


typedef struct
{
    char Ftrade_id[32+1];
    int Fcurtype;
	char Fspid[16+1];
    LONG Frecon_balance;
    char Frecon_day[20+1];
    LONG Fprofit;
    LONG Freward_profit;
    LONG Ftotal_profit;
    LONG Fvalid_money;
    LONG Ftplus_redem_money;
    char Flogin_ip[16+1];
    char Fsign[32+1];
    char Fcreate_time[20+1];
    char Fmodify_time[20+1];
    char Fmemo[128+1];
    char Fexplain[128+1];
    int Fstandby1;
    int Fstandby2;
    char Fstandby3[64+1];
    char Fstandby4[128+1];
    char Fstandby5[20+1];
    char Fstandby6[20+1];
	int Ffinancial_days;


    LONG Fprecheck_money;
    char Fprecheck_day[20+1];
    LONG Fprecheck_profit;  
	LONG Fstandby7;   //入账检查记录总份额  --指数基金用到

}FundProfit;


typedef struct
{
	char Ftrade_id[32+1];
    char Ffund_code[64+1];
    char Fspid[16+1];
	char Fstart_date[16+1];
    char Fend_date[16+1];
	LONG Ftotal_fee;
    LONG Fprofit;

} FundCloseTransProfit;



bool queryFundProfit(CMySQL* pMysql, FundProfit & data,  bool lock);

bool queryFundProfitList(CMySQL* pMysql, string trade_id, vector<FundProfit>& dataVec,  bool lock);

void insertFundProfit(CMySQL* pMysql, FundProfit& data );
    
void updateFundProfit(CMySQL* pMysql, FundProfit& data );


void updateTotalFeeAndProfit(CMySQL* pMysql, FundProfit& data );

void updateFundProfitForReward(CMySQL* pMysql, FundProfit& data );

bool setTotalProfit(FundProfit& data, int uid, int timeout, vector<FundProfit> & fundProfitVec);

bool getTotalProfit(FundProfit& data, int uid, vector<FundProfit> & fundProfitVec);

LONG countTotalReconBalance(CMySQL* pMysql,const string &trade_id);

bool queryFundProfitAndCheck(FundProfit& fund_profit, bool& fund_profit_exist, const char* date, LONG profit);

//将收益记录更新到CVK中
bool addTotalProfit2Cache(const FundProfit& data, int uid, int timeout);
int getTotalProfitFromCache(FundProfit& total_data, int uid, vector<FundProfit> & fundProfitVec);


#endif

