/**
  * FileName: fund_redeem_fetch_ack_service.h
  * Version :1.0
  * Date: 2015-3-12
  * Description: 基金交易服务 基金赎回确认
  */


#ifndef _FUND_REDEEM_FETCH_ACK_H_
#define _FUND_REDEEM_FETCH_ACK_H_

class FundRedeemFetchAck
{
public:
    FundRedeemFetchAck(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:
	TRPC_SVCINFO* m_request;			// 服务请求
    string m_spid;                      // 商户SPID

	CParams m_params;                   // 消息参数
	CMySQL* m_pFundCon;                // 基金数据库连接句柄

	ST_TRADE_FUND m_stTradeBuy;        // 交易记录
	ST_FUND_BIND m_fund_bind;          // 用户账号信息
	ST_BALANCE_ORDER m_balanceOrder;   // 余额流水

	
private:	
	
	 void CheckParams() throw (CException);
	 void checkToken() throw (CException);
	 void CheckFundBind() throw (CException);

	 void updateFetchArrival() throw (CException);
	 void updateBalanceFetchArrival() throw (CException);
	 void CheckFundBalanceOrder() throw (CException);
	 void UpdateFundBalanceOrder() throw (CException);

	 void updateRedeemFetchArrival() throw (CException);
	 void CheckFundTrade() throw (CException);
	 void UpdateFetchArrivalTime() throw (CException);
	 void updateCkvs();
	
};

#endif /* _FUND_REDEEM_FETCH_ACK_H_*/

