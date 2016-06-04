#ifndef _DB_FUND_CLOSE_BALANCE_ROLLLIST_H_
#define _DB_FUND_CLOSE_BALANCE_ROLLLIST_H_

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

struct FundCloseTrans;

typedef struct
{
    LONG Fbkid;		
	LONG Fclose_id;
	char Ftrade_id[32+1];
	char Ffund_code[16+1];
	char Fspid[16+1];
	LONG Fstart_balance;
	LONG Fbalance;
	LONG Ftail_balance;
	int Ftype;
	LONG Fbiz_fee;
	LONG Fbiz_tail_fee;
	int Fsubject;
	char Flistid[32+1];
	char Facc_time[20+1];
	char Fcreate_time[20+1];
	char Fmodify_time[20+1];
	char Fmemo[128+1];
	char Fexplain[128+1];
	char Fsign[64+1];

}FundCloseBalanceRolllist;

#define CLOSE_BALANCE_ROLLLIST_TYPE_SAVE 1 //	入
#define CLOSE_BALANCE_ROLLLIST_TYPE_DRAW 2 //	出
#define CLOSE_BALANCE_ROLLLIST_TYPE_FREEZE 3 //	冻结
#define CLOSE_BALANCE_ROLLLIST_TYPE_UNFREEZE 4 //	解冻


void insertFundCloseBalanceRolllist(CMySQL* pMysql, FundCloseBalanceRolllist& data, unsigned long long&  mysqlInsertId);

void recordFundCloseBalanceRolllist(FundCloseTrans& data,char* listid, LONG& totalFee, LONG& tailFee, int type,int subject, unsigned long long&  mysqlInsertId);


#endif

