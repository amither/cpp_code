#ifndef _DB_FUND_UNCONFIRM_H_
#define _DB_FUND_UNCONFIRM_H_

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
class FUND_UNCONFIRM
{

public:
	LONG    Fid;
    char    Ftrade_id[64+1];
    char    Fspid[16+1];     // 商户号
    char    Ftrade_date[20+1];
	char    Fconfirm_date[20+1];
	LONG    Ftotal_fee;          // 未确认金额
	LONG    Fcfm_total_fee;      // 已确认金额
	LONG    Fcfm_units;          //  已确认份额
	LONG    Funuse_units;        //  不可使用份额
	char    Ffund_net[16+1];        //  净值
	int     Fstate;
	int     Flstate;
    char    Fsign[32+1];    
    char	Fmemo[128+1];   
    char    Fcreate_time[20+1];  // 记录创建时间
    char    Fmodify_time[20+1];  // 记录最新修改时间
    
    FUND_UNCONFIRM()
		:Ftrade_id(),Fspid(),Ftrade_date(),Fconfirm_date(),Ffund_net(),Fsign(),Fmemo(),Fcreate_time(),Fmodify_time()
    {
    	Fid = MIN_INTEGER;
    	Ftotal_fee = MIN_INTEGER;
    	Fcfm_total_fee = MIN_INTEGER;
    	Fstate = MIN_INTEGER;
    	Flstate = MIN_INTEGER;
		Fcfm_units = MIN_INTEGER;
		Funuse_units = MIN_INTEGER;
    }
}; 



#define UNCONFIRM_FUND_STATE_ALL 1   // 全部未确认
#define UNCONFIRM_FUND_STATE_PART 2  // 部分未确认
#define UNCONFIRM_FUND_STATE_UNUSABLE 3  // 已确认,部分可用
#define UNCONFIRM_FUND_STATE_NONE 4  // 全部已确认

#define UNCONFIRM_FUND_VALID  1  //有效
#define UNCONFIRM_FUND_INVALID 2 // 无效

bool queryFundUnconfirm(CMySQL* pMysql,  FUND_UNCONFIRM &data,  bool islock = false);

int queryValidFundUnconfirmByTradeId(CMySQL* pMysql, const char* tradeId, vector<FUND_UNCONFIRM> &dataVec);

void updateFundUnconfirm(CMySQL* pMysql,  FUND_UNCONFIRM &data);

// 查询是否存在未确认资产份额 
bool queryFundUnconfirmExists(CMySQL* pMysql, const string& spid, const string& tradeId);

void genSign(FUND_UNCONFIRM &data);

void updateFundUnconfirmById(CMySQL* pMysql,  FUND_UNCONFIRM &data);

void insertFundUnconfirm(CMySQL* pMysql, FUND_UNCONFIRM &data );

bool setFundUnconfirm(CMySQL* mysql, const string& tradeId);

void packFundUnconfirm(vector<FUND_UNCONFIRM>& fundUnconfirmVec, string& szValue);

#endif

