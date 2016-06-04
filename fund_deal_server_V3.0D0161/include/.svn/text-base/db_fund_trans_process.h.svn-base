#ifndef _DB_FundTransProcess_H_
#define _DB_FundTransProcess_H_

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
 * 理财通交易过程表数据结构
*/
class FundTransProcess
{

public:
	LONG    Fid;
	char    Flistid[32+1];
	char    Ftrade_id[64+1];
	char    Fspid[16+1];     // 商户号
	char    Ffund_code[20+1];
	int     Ftype;
	char    Ftrade_date[20+1];
	char    Fconfirm_date[20+1];
	int     Fpur_type;
	int     Fpurpose;
	LONG    Ftotal_fee;          // 交易金额
	LONG    Ffund_units;         // 交易份额
	char    Ffund_net[16+1];     // 净值
	int     Fstate;
	int     Flstate;
	char    Fsign[32+1];    
	char    Fmemo[128+1];   
	char    Fcreate_time[20+1];  // 记录创建时间
	char    Fmodify_time[20+1];  // 记录最新修改时间
	char    Facc_time[20+1];  // 交易时间
	char    Fsubacc_time[20+1];  // 子账户交易时间
	char    Fconfirm_time[20+1];  // 确认时间
	char    Ffetch_time[20+1];  // 赎回发起提现时间
	char    Ffinish_time[20+1];  // 交易完成时间(索引),完成之前时间为9999-99-99 23:59:59
    
	FundTransProcess()
	:Flistid(),Ftrade_id(),Fspid(),Ffund_code(),Ftrade_date(),Fconfirm_date(),Ffund_net(),Fsign(),Fmemo(),
		Fcreate_time(),Fmodify_time(),Facc_time(),Fsubacc_time(),Fconfirm_time(),Ffetch_time(),
		Ffinish_time()
    {
    	Fid = MIN_INTEGER;
		Ftype = MIN_INTEGER;
    	Fpur_type = MIN_INTEGER;
    	Fpurpose = MIN_INTEGER;
    	Ftotal_fee = MIN_INTEGER;
    	Ffund_units = MIN_INTEGER;
    	Fstate = MIN_INTEGER;
    	Flstate = MIN_INTEGER;
    }
}; 

#define PROCESS_TRANS_STATE_BUY_UNCONFIRM 1   // 申购未确认
#define PROCESS_TRANS_STATE_BUY_CONFIRM 2  // 申购份额确认
#define PROCESS_TRANS_STATE_BUY_USABLE 3  // 申购份额可用(终态)
#define PROCESS_TRANS_STATE_BUY_CONFIRM_FAIL 10  //申购份额确认失败(终态)

#define PROCESS_TRANS_STATE_REDEEM_UNCONFIRM 21   // 赎回未确认
#define PROCESS_TRANS_STATE_REDEEM_CONFIRM 22  // 赎回份额确认
#define PROCESS_TRANS_STATE_REDEEM_FETCH 23  // 赎回发起提现
#define PROCESS_TRANS_STATE_REDEEM_ARRIVAL 24  // 赎回提现到账(终态)

#define PROCESS_TRANS_STATE_REDEEM_INFO_FAIL 30  //赎回通知失败(终态)
#define PROCESS_TRANS_STATE_REDEEM_CONFIRM_FAIL 31  //赎回份额确认失败(终态)
#define PROCESS_TRANS_STATE_REDEEM_FETCH_FAIL 32  //赎回提现失败(终态)


#define PROCESS_TRANS_LSTATE_VALID  1  //有效
#define PROCESS_TRANS_LSTATE_INVALID 2 // 无效

/**
 * 用户未完成资产数据结构
*/
class FundUnfinishAssert
{

public:
	char    Ftrade_id[64+1];
	char    Fspid[16+1];     // 商户号
	char    Ffund_code[20+1];
	LONG    Ftotal_fee;          // 交易金额
    
	FundUnfinishAssert()
	:Ftrade_id(),Fspid(),Ffund_code()
    {
    	Ftotal_fee = 0;
    }
}; 

bool queryFundTransProcess(CMySQL* pMysql,  FundTransProcess &data,  bool islock = false);

int queryUnfinishTransByTradeId(CMySQL* pMysql, const char* tradeId, vector<FundTransProcess> &dataVec);

void updateFundTransProcessWithSign(CMySQL* pMysql, FundTransProcess &data, FundTransProcess &dbData);

void updateFundTransProcess(CMySQL* pMysql,  FundTransProcess &data);

// 查询是否存在未确认指数 ,兼容期间增加查询unfirm_fund
bool queryUnfinishTransExistsBySp(CMySQL* pMysql, const string& spid, const string& tradeId);

// 查询是否存在未确认指数 ,兼容期间增加查询unfirm_fund
bool queryUnfinishTransExists(CMySQL* pMysql, const string& tradeId);

void insertFundTransProcess(CMySQL* pMysql, FundTransProcess &data );

int queryUnfinishTransByTradeId4CKV(CMySQL* pMysql, const char* tradeId, vector<FundTransProcess> &dataVec);

bool setFundUnfinishTransCKV(CMySQL* mysql, const string& tradeId);

void packFundUnfinishTransCKV(vector<FundTransProcess>& fundUnfinishVec, string& szValue);

int statUnfinishBuyAssetByTradeId(CMySQL* pMysql, const char* tradeId, map<string,FundUnfinishAssert> &dataMap);


#endif

