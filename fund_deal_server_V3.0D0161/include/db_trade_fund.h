#ifndef _DB_TRADE_FUND_H_
#define _DB_TRADE_FUND_H_

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


bool statCloseBuyTransFee(CMySQL* mysql, ST_TRADE_FUND& pstRecord, const string& start_day, const string& end_day);

void InsertCloseBookTradeFund(CMySQL* mysql, ST_TRADE_FUND* pstRecord);

void InsertCloseBookTradeUserFund(CMySQL* mysql, ST_TRADE_FUND* pstRecord);

bool isCloseInitRedemExists(CMySQL* mysql, const LONG closeId, const int uid, const char* fundCode,const char* startTime, const char* endTime);

bool statOpenRedeemModifyTime(CMySQL* mysql, const string& curDay,const int uid,map<string,string>& redeemTimeMap);

   
#endif

