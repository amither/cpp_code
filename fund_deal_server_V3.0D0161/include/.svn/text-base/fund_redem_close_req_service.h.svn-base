/**
  * FileName: fund_redem_close_req_service.h
  * Author: jessiegao
  * Version :1.0
  * Date: 2014-9-6
  * Description: 基金交易服务 定期产品实时赎回请求
  */


#ifndef _FUND_DEAL_REDEM_CLOSE_REQ_H_
#define _FUND_DEAL_REDEM_CLOSE_REQ_H_

class FundRedemCloseReq
{
public:
    FundRedemCloseReq(CMySQL* mysql);

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

    void CheckFundTrade() throw (CException);
	void CheckFundTradeRepeat() throw (CException);
    void RecordFundTrade();

	void CheckAuthLimit() throw (CException);

	
private:

    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

    ST_FUND_BIND m_fund_bind;           // 用户账号信息
    FundBindSp m_fund_bind_sp_acc;	
	FundSpConfig m_fund_sp_config;
	ST_TRADE_FUND m_stTradeRedem; 	// 交易记录
	FundCloseTrans m_close_trans;

	bool m_redemTradeExist;

};

#endif /* _FUND_DEAL_REDEM_CLOSE_REQ_H_*/

