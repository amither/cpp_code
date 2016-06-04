#ifndef _DB_FUND_CONFIG_H_
#define _DB_FUND_CONFIG_H_

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
    char Fkeyword[128+1];
    char Fvalue[1024+1];
    char FcreateTime[20+1];
    char FmodifyTime[20+1];
    char Fmemo[128+1];
}FundConfig;




bool queryFundConfig(CMySQL* pMysql, FundConfig & data,  bool lock);

void insertFundConfig(CMySQL* pMysql, FundConfig & data);

   
#endif

