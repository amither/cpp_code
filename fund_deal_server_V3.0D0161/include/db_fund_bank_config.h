#ifndef _DB_FUND_BANK_CONFIG_H_
#define _DB_FUND_BANK_CONFIG_H_

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
    int Fbank_type;
	char Fbank_abbr[16+1];
    char Fbank_name[64+1];
    int Flstate;
    int Fsupport_type;
    LONG Fonce_quota;
    LONG Fday_quota;
    int Farrival_type;
	char Farrival_time[64+1];
    char Fcreate_time[20+1];
    char Fmodify_time[20+1];
    char Fmemo[128+1];
	char Fbank_area[16+1];
	char Fbank_city[16+1];
}FundBankConfig;




bool queryFundBankConfig(CMySQL* pMysql, FundBankConfig & data,  bool lock);

bool queryFundAllBankConfig(CMySQL* pMysql, vector<FundBankConfig>& dataVec,  bool lock);

void updateFundBankConfig(CMySQL* pMysql, FundBankConfig& data );

bool setSupportBankToKV(FundBankConfig& data);

bool setAllSupportBankToKV(CMySQL* pMysql);

bool getSupportBankFromKV(int iBankType, string &sBankName);
   
#endif

