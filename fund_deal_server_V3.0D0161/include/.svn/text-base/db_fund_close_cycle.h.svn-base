#ifndef _DB_FUND_CLOSE_CYCLE_H_
#define _DB_FUND_CLOSE_CYCLE_H_

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
    char Fdate[20+1];
    char Ffund_code[64+1];
    char Ftrans_date[16+1];
    char Ffirst_profit_date[16+1];
    char Fopen_date[16+1];
    char Fbook_stop_date[16+1];
    char Fstart_date[16+1];
    char Fdue_date[16+1];
    char Fprofit_end_date[16+1];
    int Flstate;
    char Fmemo[128+1];
    char Fcreate_time[20+1];
    char Fmodify_time[20+1];
    int Fstandby1;
    int Fstandby2;
    char Fstandby3[64+1];
    char Fstandby4[128+1];
    char Fstandby5[20+1];
    char Fstandby6[20+1];
}FundCloseCycle;




bool queryFundCloseCycle(CMySQL* pMysql, FundCloseCycle & data,  bool lock);

bool queryFundCloseCycle(CMySQL* pMysql,int offset,int limit,FundCloseCycle &where,vector< FundCloseCycle>& dataVec,  bool lock);

bool setAllCloseFundCycleToKV();

string getCacheCloseMaxTradeDate(CMySQL* pMysql, const string& curDate, const string& fund_code);

int getCloseMaxTradeDate(CMySQL* pMysql, FundCloseCycle & cycle);

string getCacheCloseMinNatureDate(CMySQL* pMysql, const string& curDate, const string& fund_code);

int getCloseMinNatureDate(CMySQL* pMysql, FundCloseCycle& data);

   
#endif

