#ifndef FUND_CHECK_CKV_SERVICE_H
#define FUND_CHECK_CKV_SERVICE_H

#include "fund_commfunc.h"

struct ckv_result_t
{
    string key;
    int result;
    string value;
};

enum FUND_CKV_CHECK_TYPE {
    FUND_CKV_CHK_TYPE_INC_CHK = 1,//增量数据核对
    FUND_CKV_CHK_TYPE_OVERALL = 2 //全量数据核对
};

class FundCheckCkv
{
public:
	FundCheckCkv(CMySQL* mysql);
	~FundCheckCkv();
	void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);
	void checkParams() throw (CException);
private:	
	void checkToken() throw (CException);
	void overAllCheck(int key_no, const ckv_result_t &ckv_res, const string &no_prefix_key);
	void incCheck(int key_no, const ckv_result_t &ckv_res, const string &no_prefix_key);
	
	void allChkUsrFundBind(const ckv_result_t &ckv_res, const string &uin);
	void allChkUsrBindSp(const ckv_result_t &ckv_res, const string &trade_id);
	void allChkUsrPayCard(const ckv_result_t &ckv_res, const string &uin);
	void allChkUsrCloseTrans(const ckv_result_t &ckv_res, const string &tradeid_fundcode);
	void allChkUsrFundTrade(const ckv_result_t &ckv_res, const string &tradeid);
	void allChkUnconfirm(const ckv_result_t &ckv_res, const string &tradeid);
	void allChkUnfinishTrans(const ckv_result_t &ckv_res, const string &tradeid);
	void allChkCashInTransit(const ckv_result_t &ckv_res, const string &tradeid);

	void incChkUsrProfitRecord(const ckv_result_t &ckv_res, const string &trade_id);
private:
	CMySQL* m_pSlaveFundDb; // 基金数据库连接句柄
	CParams m_params;
};

#endif /* FUND_CHECK_CKV_SERVICE_H */
