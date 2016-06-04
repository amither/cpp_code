#ifndef _DB_FUND_DYNAMIC_H_
#define _DB_FUND_DYNAMIC_H_

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
    char Fredem_day[20+1];
    int Fredem_times_day;
    int Fdyn_status_mask;
    char Fmodify_time[20+1];
    char Fcreate_time[20+1];
    int Flstate;

}ST_FUND_DYNAMIC;


bool queryFundDynamic(CMySQL* pMysql, ST_FUND_DYNAMIC& data,  bool lock);

void insertFundDynamic(CMySQL* pMysql, ST_FUND_DYNAMIC& data );

void updateFundDynamic(CMySQL* pMysql, ST_FUND_DYNAMIC& data );

bool setUserDynamicInfoToCkv(ST_FUND_DYNAMIC& data,bool queryDb);

bool setUserDynamicInfoToCkv(CMySQL* pMysql,ST_FUND_DYNAMIC& data,bool queryDb);
   
#endif