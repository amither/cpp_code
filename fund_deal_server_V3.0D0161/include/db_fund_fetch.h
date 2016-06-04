#ifndef _DB_FUND_FETCH_H_
#define _DB_FUND_FETCH_H_

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
    char Ffetchid[32+1];
    char Ffund_trans_id[32+1];
    char Fcreate_time[20+1];
    char Fmodify_time[20+1];
}FundFetch;




bool queryFundFetch(CMySQL* pMysql, FundFetch & data,  bool lock);



void insertFundFetch(CMySQL* pMysql, FundFetch& data );

   
#endif

