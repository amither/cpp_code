#ifndef _DB_FUND_TRANS_DATE_H_
#define _DB_FUND_TRANS_DATE_H_

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
    char Fdate[20+1];
    char Fspid[16+1];
    int Fstate;
    char Fmemo[128+1];
    char Fcreate_time[20+1];
    char Fmodify_time[20+1];
    int Fstandby1;
    int Fstandby2;
    char Fstandby3[64+1];
    char Fstandby4[128+1];
    char Fstandby5[20+1];
    char Fstandby6[20+1];
}FundTransDate;

struct FundLastAndNextTransDate {
	char Fdate[20+1];
    char Fspid[16+1];
	char Flast_date[20+1];
	char Fnext_date[20+1];
	char Fstandby3[64+1];
	char Fmemo[128+1];
	int Fstandby1;
};


bool queryFundTransDate(CMySQL* pMysql, FundTransDate & data,  bool lock);

//≤È—Øt+n»’∆⁄
bool queryFundTplusNDates(CMySQL* pMysql, vector<string>& dateVec, string curTime,int n);

bool queryFundProfitDate(CMySQL* pMysql,  FundTransDate& data, bool lock);

string getCacheTradeDate(CMySQL* pMysql, const string& curTime);

string getCacheT1Date(CMySQL* pMysql, const string& curTime);

string getTradeDate(CMySQL* pMysql, const string& curTime);

void getTradeDate(CMySQL* pMysql, const string& curTime , string& trade_date, string& fund_vdate);

void getTplusFetchDate(CMySQL* pMysql, const string& curTime , string& fund_fetch_date);

void getTminus2TransDate(CMySQL* pMysql, const string& curData , string& Tminus2Date,string &TminusDate,bool &isCurTDay);

string calculateFundDate(const string& curTime);

void queryLastAndNextDate(CMySQL* pMysql, const string &curData, vector<FundLastAndNextTransDate>& list, bool limit1=false);

bool setFundTransDay2Ckv(CMySQL* pMysql, const string &cur_day);

void insertTransDate(CMySQL* pMysql, FundTransDate& data);

void updateTransDate(CMySQL* pMysql, FundTransDate &data);

void queryHKLastAndNextDate(CMySQL* pMysql, const string &curDate, vector<FundLastAndNextTransDate>& list, bool limit1);

bool queryExactTransDate(CMySQL* pMysql, FundTransDate& data, bool lock);

bool queryHKTransDate(CMySQL* pMysql, FundTransDate& data, bool lock);

void queryFundTDateforCache(CMySQL* mysql, string sDay, string &sData, string &sHKData) throw (CException);

#endif

