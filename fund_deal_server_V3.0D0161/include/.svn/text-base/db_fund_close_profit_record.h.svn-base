#ifndef _DB_FUND_CLOSE_PROFIT_RECORD_H_
#define _DB_FUND_CLOSE_PROFIT_RECORD_H_

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
    LONG Fid;		
    char Flistid[32+1];
	LONG Fclose_id;
	char Ftrade_id[32+1];
	char Fend_date[16+1];
	char Fprofit_end_date[16+1];
	char Fday[16+1];
	char Ffund_code[16+1];
	LONG Ftotal_fee;
	LONG Fprofit;
	LONG Fend_tail_profit;
	int Fprofit_type;
	LONG F1day_profit_rate;
	LONG F7day_profit_rate;
	char Flogin_ip[16+1];
	char Fsign[32+1];
    char Fcreate_time[20+1];
    char Fmodify_time[20+1];
}FundCloseProfitRecord;



bool queryFundCloseProfitRecord(CMySQL* pMysql, FundCloseProfitRecord & data,  bool lock);

LONG statFundCloseReconBalance(CMySQL* pMysql, const string& date, const char* trade_id, const LONG closeId, const LONG lastCloseId);

void insertFundCloseProfitRecord(CMySQL* pMysql, FundCloseProfitRecord& data );
   
#endif

