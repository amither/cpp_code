#ifndef _DB_FUND_RECON_LOG_H_
#define _DB_FUND_RECON_LOG_H_

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
    int Frecon_type;
    char Fspid[16+1];
    char Frecon_date[20+1];
    int Frecon_state;
    int Fsuccess_count;
    LONG Fsuccess_money;
    char Fmemo[1024+1];
    char Fcreate_time[20+1];
    char Fmodify_time[20+1];
    int Fstandby1;
    int Fstandby2;
    char Fstandby3[64+1];
    char Fstandby4[128+1];
    char Fstandby5[20+1];
    char Fstandby6[20+1];
}FundReconLog;




bool queryFundReconLog(CMySQL* pMysql, FundReconLog & data,  bool lock);

void insertFundReconLog(CMySQL* pMysql, FundReconLog& data );
  
void updateFundReconLog(CMySQL* pMysql, FundReconLog& data );

bool queryFundSpReconLog(CMySQL* pMysql, FundReconLog& data,  bool lock=false);

// bool queryFundReconLogFinishAll(CMySQL* pMysql, FundReconLog& data);

   
#endif

