/**
  * FileName: fund_close_roll_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2014-7-30
  * Description: 定期记录到期滚动
  */


#ifndef _FUND_DEAL_CLOSE_ROLL_H_
#define _FUND_DEAL_CLOSE_ROLL_H_

class FundCloseRoll
{
public:
    FundCloseRoll(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
	void CheckCloseTrans() throw (CException);
	void CheckCloseCycle() throw (CException);
	void checkNextCloseFundTransSeq() throw (CException);
	void updateCkvs();
	void closeRoll() throw (CException);
	void recordSpScope() throw (CException);

	
private:
	TRPC_SVCINFO* m_request;			// 服务请求
    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

	FundCloseTrans m_close_trans;   // 待滚动定期交易数据
	FundCloseTrans m_next_close_trans;   // 滚动后定期交易数据
	FundCloseCycle m_cycle;     // 定期交易周期
	int m_seqno; // 滚动序列号
	LONG m_roll_fee; // 滚动金额
	bool m_hasRollTrans; //存在滚动期次 

};

#endif

