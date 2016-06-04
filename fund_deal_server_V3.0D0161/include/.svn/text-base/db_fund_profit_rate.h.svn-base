#ifndef _DB_FUND_PROFIT_RATE_H_
#define _DB_FUND_PROFIT_RATE_H_

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
#include "fund_commfunc.h"


typedef struct
{
    LONG Fimt_id;
    char Fspid[16+1];
    char Fdate[20+1];
    char Ffund_code[64+1];
    char Ffund_name[64+1];
    LONG F1day_profit_rate;
    LONG F7day_profit_rate;
	LONG F1week_rise_rate;
	LONG F1month_rise_rate;
	LONG F3month_rise_rate;
	LONG F6month_rise_rate;
	LONG F1year_rise_rate;
	LONG Fcumulative_net;
	LONG F1month_annual_profit;
	LONG F3month_annual_profit;
	LONG F6month_annual_profit;
	LONG F1year_track_error;
    char Fcreate_time[20+1];
    char Fmodify_time[20+1];
    int Fstandby1;
    int Fstandby2;
    char Fstandby3[64+1];
    char Fstandby4[128+1];
    char Fstandby5[20+1];
    char Fstandby6[20+1];
}FundProfitRate;


bool queryFundProfitRate(CMySQL* pMysql, FundProfitRate& data,  bool lock) ;

bool queryFundLatestProfitRate(CMySQL* pMysql, FundProfitRate & data,  bool lock);

bool queryFundProfitRate(CMySQL* pMysql,int offset, int limit, FundProfitRate &where, vector<FundProfitRate>& datavec,  bool lock);

void insertFundProfitRate(CMySQL* pMysql, FundProfitRate& data );

void updateFundProfitRate(CMySQL* pMysql, FundProfitRate& data );

bool setSpProfitRateToKV(CMySQL* pMysql, FundProfitRate& data);

bool setMultiSpProfitRateToKV(CMySQL* mysql, FundProfitRate& fundProfitRate);

bool setHighestProfitRateSpToKV(CMySQL* pMysql, FundProfitRate& data);

bool getHighestProfitRateSpFromKV(FundProfitRate &data);

bool setAllLastSpProfitRateToKV(CMySQL *mysql);

LONG getCacheETFNet(CMySQL* pMysql, const string& spid, const string& fund_code);

LONG getCacheSpNet(CMySQL* pMysql, SpConfigCache& spConfig);


#endif

