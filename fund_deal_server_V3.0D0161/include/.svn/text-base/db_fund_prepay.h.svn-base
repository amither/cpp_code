#ifndef _DB_FUND_PREPAY_H_
#define _DB_FUND_PREPAY_H_

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
    char Flistid[32+1];
    char Fuin[64+1];
    int Fuid;
    int Facct_type;
    char Fspid[16+1];
    char Ffund_name[64+1];
    char Ffund_code[64+1];
    LONG Ftotal_fee;
    char Fcft_trans_id[32+1];
    int Fbank_type;
    char Fcard_no[8+1];
    int Fstate;
	int Fauthen_state;
    char Frefund_id[32+1];
    char Frefund_time[20+1];
    char Fmemo[128+1];
    char Fcreate_time[20+1];
    char Fmodify_time[20+1];
    int Fstandby1;
    int Fstandby2;
    char Fstandby3[64+1];
    char Fstandby4[128+1];
    char Fstandby5[20+1];
    char Fstandby6[20+1];
	char Fopenid[64+1];
}FundPrepay;




bool queryFundPrepay(CMySQL* pMysql, FundPrepay & data,  bool lock);



void insertFundPrepay(CMySQL* pMysql, FundPrepay& data );

    
void updateFundPrepay(CMySQL* pMysql, FundPrepay& data );
   
#endif

