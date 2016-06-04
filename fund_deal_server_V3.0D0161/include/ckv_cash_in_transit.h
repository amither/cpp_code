#ifndef _CKV_CASH_IN_TRANSIT_H_
#define _CKV_CASH_IN_TRANSIT_H_

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


/**
 * 理财通
*/
class CashInTransit
{

public:
	char    listid[32+1];
	int     curtype;
	char    trade_id[32+1];
	LONG    total_fee;
	int     state;
	char    spid[16+1];
	char    acc_time[20+1];
    
	CashInTransit()
	:listid(),trade_id(),spid(),acc_time()
    {
    	curtype = MIN_INTEGER;
		total_fee = MIN_INTEGER;
		state=MIN_INTEGER;
    }
}; 

#define CASH_IN_TRANSIT_BUY_UNCONFIRM 1   // 申购未确认
#define CASH_IN_TRANSIT_BUY_CONFIRM 2  // 申购份额确认，不可用
#define CASH_IN_TRANSIT_REDEEM_UNCONFIRM 21   // 赎回未确认
#define CASH_IN_TRANSIT_REDEEM_CONFIRM 22  // 赎回份额确认
#define CASH_IN_TRANSIT_REDEEM_FETCH 23  // 赎回发起提现

#define CASH_IN_TRANSIT_BALANCE_T0FETCH 41  //T+0余额提现发起
#define CASH_IN_TRANSIT_BALANCE_T1FETCH 42  //T+1余额提现发起

int queryCashInTransitBAFetch4CKV(CMySQL* pMysql, const char* tradeId, vector<CashInTransit> &dataVec);

bool setCashInTransitCKV(CMySQL* mysql, const string& tradeId);

void packCashInTransitCKV(vector<CashInTransit>& cashInTransitVec, string& szValue);

#endif

