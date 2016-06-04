/**
  * FileName: fund_redem_sp_ack_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-8-19
  * Description: 基金交易服务 基金赎回确认
  */


#ifndef _FUND_DEAL_REDEM_SP_ACK_H_
#define _FUND_DEAL_REDEM_SP_ACK_H_

class FundRedemSpAck
{
public:
    FundRedemSpAck(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);;

private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);

	void CheckFundBind() throw (CException);

    void CheckFundTrade() throw (CException);
    void CheckTradeTime() throw (CException);
    void UpdateTradeState();
	void UpdateRedemTradeForSuc() throw (CException);
	void UpdateRedemTradeForTimeout() throw (CException);
	void UpdateRedemTradeForFail() throw (CException);
	void UpdateRedemTradeForFinish() throw (CException);
	void UpdateRedemTradeForBudan() throw (CException);
	void checkSpLoaning() throw (CException);
	void doDraw() throw (CException);
	void sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy);
	void updateExauAuthLimitNoExcp();
       void updateWxPrePayUserBalance()throw (CException);

private:
	TRPC_SVCINFO* m_request;			// 服务请求

    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

    ST_TRADE_FUND m_stTradeBuy; // 交易记录
    ST_FUND_BIND m_fund_bind;           // 用户账号信息
    FundUserTotalAcc m_fundUserTotalAcc; //基金总账户信息

	int  m_optype;                      // 操作类型
	int m_draw_arrive_type; //提现类型
	int m_loading_type;//是否走垫资账户
	bool m_stop_fetch;
	bool m_need_updateExauAuthLimit; //标记是否需要累加exau限额
	string m_subAccControlList; //合约机解冻提现 子账户受控单号
	bool m_subAccDrawOk;

};

#endif /* _FUND_DEAL_REDEM_SP_ACK_H_*/

