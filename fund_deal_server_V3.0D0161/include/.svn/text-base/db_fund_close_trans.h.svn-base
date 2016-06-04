#ifndef _DB_FUND_CLOSE_TRANS_H_
#define _DB_FUND_CLOSE_TRANS_H_

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

struct FundSpConfig;


class FundCloseTrans
{
public:
    LONG Fid;
    char Ftrade_id[32+1];
    char Ffund_code[64+1];
    char Fspid[16+1];
    int Fseqno;
    int Fuid;
    int Fpay_type;
    LONG Flastid;
    LONG Fstart_total_fee;
    LONG Fcurrent_total_fee;
    LONG Fend_tail_fee;
    int Fuser_end_type;
    int Fend_sell_type;
    LONG Fend_plan_amt;
    LONG Fend_real_buy_amt;
    LONG Fend_real_sell_amt;
    char Fpresell_listid[32+1];
    char Fsell_listid[32+1];
    char Fend_listid_1[32+1];
    char Fend_listid_2[32+1];
    char Fend_transfer_fundcode[64+1];
    char Fend_transfer_spid[16+1];
	char Facc_time[20+1];
    char Ftrans_date[16+1];
    char Ffirst_profit_date[16+1];
    char Fopen_date[16+1];
    char Fbook_stop_date[16+1];
    char Fstart_date[16+1];
    char Fend_date[16+1]; // 唯一日期标识
    char Fdue_date[16+1]; // 到期日
	char Fprofit_end_date[16+1];
    char Fchannel_id[64+1];
    int Fstate;
    int Flstate;
	char Fprofit_recon_date[16+1];
	LONG Flast_profit;
	LONG Ftotal_profit;
    char Fcreate_time[20+1];
    char Fmodify_time[20+1];
    char Fmemo[128+1];
    char Fexplain[128+1];
    char Fsign[64+1];
	LONG Fbkid;
	char Flastids[128+1];
    int Fstandby1;
    int Fstandby2;
    int Fstandby3;
    char Fstandby4[64+1];
    char Fstandby5[64+1];
    char Fstandby6[64+1];
    char Fstandby7[255+1];
    char Fstandby8[255+1];
    char Fstandby9[20+1];
    char Fstandby10[20+1];

	FundCloseTrans()
		:Ftrade_id(),Ffund_code(),Fspid(),Fpresell_listid(),Fsell_listid(),Fend_listid_1(),Fend_listid_2(),
		Fend_transfer_fundcode(),Fend_transfer_spid(),Facc_time(),Ftrans_date(),Ffirst_profit_date(),Fopen_date(),
		Fbook_stop_date(),Fstart_date(),Fend_date(),Fprofit_end_date(),Fchannel_id(),Fprofit_recon_date(),Fcreate_time(),
		Fmodify_time(),Fmemo(),Fexplain(),Fsign(),Flastids(),Fstandby4(),Fstandby5(),Fstandby6(),Fstandby7(),Fstandby8(),Fstandby9(),
		Fstandby10()
	{
		Fid = MIN_INTEGER;
		Flastid = MIN_INTEGER;
		Fstart_total_fee = MIN_INTEGER;
		Fcurrent_total_fee = MIN_INTEGER;
		Fend_tail_fee = MIN_INTEGER;
		Fend_plan_amt = MIN_INTEGER;
		Fend_real_buy_amt = MIN_INTEGER;
		Fend_real_sell_amt = MIN_INTEGER;
		Flast_profit = MIN_INTEGER;
		Fseqno = MIN_INTEGER;
		Fuid = MIN_INTEGER;
		Fpay_type = MIN_INTEGER;
		Fuser_end_type = MIN_INTEGER;
		Fend_sell_type = MIN_INTEGER;
		Fstate = MIN_INTEGER;
		Flstate = MIN_INTEGER;
		Fstandby1 = MIN_INTEGER;
		Fstandby2 = MIN_INTEGER;
		Fstandby3 = MIN_INTEGER;
		Ftotal_profit = MIN_INTEGER;
		Fbkid = MIN_INTEGER;
	};
	
};

/* 定期记录支付类型 */
#define CLOSE_FUND_PAY_TYPE_WX 0x1 //	表示微信支付
#define CLOSE_FUND_PAY_TYPE_EXTENSION 0x2 //	表示到期顺延申购

/* 用户指定的到期申购/赎回策略 */
#define CLOSE_FUND_END_TYPE_PATRIAL_REDEM 1 //	指定赎回金额
#define CLOSE_FUND_END_TYPE_ALL_REDEM 2 //	全额赎回
#define CLOSE_FUND_END_TYPE_ALL_EXTENSION 3 //	全额顺延(默认)
#define CLOSE_FUND_END_TYPE_ALL_KEFU 99 //	客服全额强制赎回


/* 用户指定的赎回类型 */
#define CLOSE_FUND_SELL_TYPE_BANK_CARD 1	// 赎回用于提现到银行卡
#define CLOSE_FUND_SELL_TYPE_ANOTHER_FUND 2	// 赎回用于转换另一只基金
#define CLOSE_FUND_SELL_TYPE_BALANCE 3	// 赎回用于转余额账户

/* 定期交易记录状态 */
#define CLOSE_FUND_STATE_INIT 1			// 1：初始状态（暂未使用）
#define CLOSE_FUND_STATE_PENDING 2		//2：待执行（申购支付完成即该状态）
#define CLOSE_FUND_STATE_PROCESSING 3	//3：该状态不再使用
#define CLOSE_FUND_STATE_REDEM_SUC 4	//4：预约赎回成功
#define CLOSE_FUND_STATE_SUC 5			//5：执行成功（包括到期赎回完成、到期延期完成、通过客服提前赎回完成，最终状态）
#define CLOSE_FUND_STATE_FAIL 6			//6：执行失败
#define CLOSE_FUND_STATE_PROFIT_END 7	//7：入账结束等待滚动

//缓存定期记录的最大条数
#define CACHE_CLOSE_TRANS_MAX_NUM 61

bool queryFundCloseTransForRegProfit(CMySQL* pMysql,const string& trade_id, const string& fund_code,const string& date, vector< FundCloseTrans>& dataVec,  bool lock);

/*
* query返回多行数据函数
*
* 此接口只能定期更新CKV调用
*/
bool queryFundCloseTransWithProfitEndDate(CMySQL* pMysql,int offset,int limit,FundCloseTrans &where,vector< FundCloseTrans>& dataVec,  bool lock);

int queryValidFundCloseTransCount(CMySQL* pMysql, const FundCloseTrans &where, const string &trade_date);

bool queryFundCloseTrans(CMySQL* pMysql, FundCloseTrans& data,  bool lock);

bool queryLatestFundCloseTrans(CMySQL* pMysql, FundCloseTrans& data,  bool lock);

void insertFundCloseTrans(CMySQL* pMysql, FundCloseTrans& data, unsigned long long&  mysqlInsertId );
  
void updateFundCloseTrans(CMySQL* pMysql, FundCloseTrans& data );

int checkPermissionBuyCloseFund(FundCloseTrans& data, const FundSpConfig& fundSpConfig, string trans_date, string close_due_date, bool lock);

void checkPermissionBuyCloseFund(const string trade_id, const FundSpConfig& fundSpConfig, const string systime, bool lock);

bool setFundCloseTransToKV(const string &trade_id, const string &fund_code);

/**
 * 根据数据组装定期记录ckv的值
 * @param list 记录列表
 * @param value ckv中存储的值
 * @return 0-成功，其他-失败
 */
int packFundCloseTransCkvValue(const vector< FundCloseTrans> &list, string &value);

/**
 * 解析CKV中用户定期交易记录
 * @param value ckv中了字串
 * @param list 解析后的定期交易记录
 * @return 0-成功，其他-失败
 */
int parseFundCloseTransCkvValue(const string &value, vector< FundCloseTrans> &list);

bool setFundCloseTransToKV(vector< FundCloseTrans>& fundCloseTransVec);

bool getFundCloseTransKV(string trade_id, string fund_code,vector<FundCloseTrans> & fundCloseTransVec);

/**
*设置cache，此接口只能定期收益入账调用
*/
bool addFundCloseTransToKV(vector< FundCloseTrans>& fundCloseTransVec);

bool queryFundCloseTransByEndDate(CMySQL* pMysql, FundCloseTrans& data, bool lock);

void updateFundCloseTransById(CMySQL* pMysql, FundCloseTrans& data );

void saveFundCloseTrans(FundCloseTrans& data,FundCloseTrans& dbData,const char* listid,const int subject);

void createFundCloseTrans(FundCloseTrans& data,const char* listid,const int subject);

bool queryFundCloseTransAllByProfitEnd(CMySQL* pMysql,const string& trade_id,int date, vector< FundCloseTrans>& dataVec,  bool lock=false);

#endif

