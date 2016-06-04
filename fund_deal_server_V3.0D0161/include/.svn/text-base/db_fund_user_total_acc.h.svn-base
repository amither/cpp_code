#ifndef _DB_FUND_USER_TOTAL_ACC_H_
#define _DB_FUND_USER_TOTAL_ACC_H_

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
    char Ftrade_id[32+1];
    int Fbusiness_type;
    LONG Fbalance;
    LONG Ffreeze;
    LONG Funfreeze;
    int Fstate;
    int Flstate;
    char Flogin_ip[16+1];
	LONG Fbkid;
    char Fsign[32+1];
    char Fcreate_time[20+1];
    char Fmodify_time[20+1];
    char Fmemo[128+1];
    char Fexplain[128+1];
    int Fstandby1;
    int Fstandby2;
    int Fstandby3;
    char Fstandby4[64+1];
    char Fstandby5[64+1];
    char Fstandby6[64+1];
    char Fstandby7[255+1];
    char Fstandby8[255+1];
    char Fstandby9[20+1];
    char Fstandby10[20+1];
}FundUserTotalAcc;

LONG queryFundUserTotalAccBalanceFromSubaccCkv(const string & trade_id, int uid);

LONG queryUserTotalAsset(int uid, const string tradeId, bool addBalance=true, bool addFreeze=true);
   
#endif

