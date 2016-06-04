#ifndef _DB_FUND_PROFIT_RECORD_H_
#define _DB_FUND_PROFIT_RECORD_H_

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
    char Fsub_trans_id[32+1];
    char Ftrade_id[32+1];
    int Fcurtype;
    char Fspid[16+1];
    LONG Fvalid_money;
    LONG Fstop_money;
    LONG Ftplus_redem_money;
    LONG Frecon_balance;
    LONG Fprofit;
	LONG Ftotal_profit;
    char Fday[20+1];
    int Fprofit_type;
    LONG F1day_profit_rate;
    LONG F7day_profit_rate;
    char Flogin_ip[16+1];
    char Fsign[32+1];
    char Fcreate_time[20+1];
    char Fmodify_time[20+1];
    char Fmemo[128+1];
    LONG Fend_tail_fee;
    int Fstandby1;
    int Fstandby2;
    char Fstandby3[64+1];
    char Fstandby4[128+1];
    char Fstandby5[20+1];
    char Fstandby6[20+1];
}FundProfitRecord;


#define FUND_PROFIT_NORMAL 1 //1：份额收益（默认）
#define FUND_PROFIT_REWARD 2 //2：赠送收益
#define FUND_PROFIT_END 3 //3：扫尾收益


//缓存用户收益流水的最大条数
#define CACHE_USR_PROFIT_REC_MAX_NUM 21



bool queryFundProfitRecord(CMySQL* pMysql, FundProfitRecord & data,  bool lock);

bool queryFundProfitRecord(CMySQL* pMysql,int offset,int limit,FundProfitRecord &where,vector< FundProfitRecord>& datavec,  bool lock);

bool queryFundProfitRecordByTime(CMySQL* pMysql, const string &trade_id, const string &start_time, 
    const string &end_time, vector<FundProfitRecord>& datavec);

void insertFundProfitRecord(CMySQL* pMysql, FundProfitRecord& data );

bool setFundProfitRecordToKV(CMySQL* mysql, FundProfitRecord& fundProfitRecord);

int getFundProfitRecordFromCache(const string trade_id, vector<FundProfitRecord> &list);

bool addFundProfitRecordToCache(const FundProfitRecord& fundProfitRecord);

/**
 * 解析ckv中收益流水数据
 * @param value ckv中的收益流水列表值
 * @param list 解析后的列表
 * @return 0-成功，其它-失败
 */
int parseProfitRecordCkvValue(const string &value, vector<FundProfitRecord> &list);
   
#endif

