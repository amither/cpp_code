#ifndef _DB_CHANGE_SP_RECORD_H_
#define _DB_CHANGE_SP_RECORD_H_

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
    char Ftrade_id[32+1];
    char Fori_spid[16+1];
    char Fnew_spid[16+1];
    char Fbuy_id[32+1];
    char Fredem_id[32+1];
	char Fcft_bank_billno[32+1];
    int  Fstate;
    char Fcreate_time[20+1];
    char Fmodify_time[20+1];
}ChangeSpRecord;




bool queryChangeSpRecord(CMySQL* pMysql, ChangeSpRecord & data,  bool lock);

bool queryChangingSpRecord(CMySQL* pMysql, ChangeSpRecord& data,  bool lock);


void insertChangeSpRecord(CMySQL* pMysql, ChangeSpRecord& data );

    
void updateChangeSpRecord(CMySQL* pMysql, ChangeSpRecord& data );

// bool checkCanChangeTradeSp(CMySQL* pMysql,bool throwExp = true);

   
#endif

