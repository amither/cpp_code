/**
  * FileName: fund_refund_service.h
  * Version :1.0
  * Date: 2015-3-3
  * Description: 基金交易退款接口
  */


#ifndef _FUND_DEAL_REFUND_H_
#define _FUND_DEAL_REFUND_H_


class FundRefund
{
public:
    FundRefund(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);;

private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
    void CheckFundBind() throw (CException);

    void CheckFundTrade() throw (CException);
    void CheckSpConfig();
    void CheckFundUnconfirm();
    void UpdateFundUnconfirm();
    void UpdateTradeRefund();
    void UpdateTradeRefundForReq();
    void UpdateTradeRefundForAck();
    void CheckTradeStateForReq();
    void CheckTradeStateForAck();
    void UpdateTradeStateForReq();
    void UpdateTradeStateForAck();
	void updateCkvs();

private:
	TRPC_SVCINFO* m_request;			// 服务请求
    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄
    
    ST_FUND_BIND m_fund_bind;           // 用户账号信息
    ST_TRADE_FUND m_stTradeBuy; // 购买记录
    FundSpConfig m_fundSpConfig;
	FUND_UNCONFIRM m_fundUnconfirm;
	FundTransProcess m_fundIndexTrans;
	bool hasTransProcess;

};

#endif 

