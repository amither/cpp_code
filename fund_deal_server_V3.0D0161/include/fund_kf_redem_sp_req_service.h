/**
  * FileName: fund_redem_sp_req_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-8-19
  * Description: 基金交易服务 基金赎回请求
  */


#ifndef _FUND_DEAL_KF_REDEM_SP_REQ_H_
#define _FUND_DEAL_KF_REDEM_SP_REQ_H_

class FundKfRedemSpReq
{
public:
    FundKfRedemSpReq(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);

    void CheckFundBind() throw (CException);
    void CheckFundBindSpAcc() throw (CException);
    void CheckFundBalance();
    void checkSpLoaning() throw (CException);
	void checkSpLoaningPlus() throw (CException);

    void CheckFundTrade() throw (CException);
    void RecordFundTrade();
	void GenerFundTrade();
	void updateCkvs();
	void updateExauAuthLimitNoExcp();
	void UpdateFundCloseState();
	void UpdateState();
	void doDraw() throw (CException);
	void RecordRedemTradeForSuc() throw (CException);
	void sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy);

	bool CreatTradeInfo();
	void ProcessTradeInfo();

	void QueryFundCloseId();
	void UpdateFundCloseListid();

	
private:
	TRPC_SVCINFO* m_request;			// 服务请求
    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

    ST_FUND_BIND m_fund_bind;           // 用户账号信息
    FundBindSp m_fund_bind_sp_acc;	
    FundSpConfig m_fund_sp_config;
    ST_TRADE_FUND m_stTradeBuy; 	
    ST_TRADE_FUND  m_stRecord;  // 交易记录
    FundCloseTrans m_fund_close; // 定期交易记录
    FundUserTotalAcc m_fundUserTotalAcc; //基金总账户信息
    
    bool m_bBuyTradeExist; //赎回请求记录是否存在
	LONG m_real_fee;	//本金
	LONG m_total_fee;	//本金+收益

};

#endif /* _FUND_DEAL_REDEM_SP_REQ_H_*/

