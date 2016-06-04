/**
  * FileName: abstract_redeem_sp_ack_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-3-12
  * Description: 基金交易服务 基金赎回确认
  */


#ifndef _ABSTRACT_REDEM_SP_ACK_H_
#define _ABSTRACT_REDEM_SP_ACK_H_

class AbstractRedeemSpAck
{
public:
    AbstractRedeemSpAck(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst,char* szMsg) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);
    void setSpConfig(FundSpConfig fundSpConfig);

protected:
	TRPC_SVCINFO* m_request;			// 服务请求

	CParams m_params;                   // 消息参数
	CMySQL* m_pFundCon;                // 基金数据库连接句柄

	ST_TRADE_FUND m_stTradeBuy; // 交易记录
	ST_FUND_BIND m_fund_bind;           // 用户账号信息
	FundUserTotalAcc m_fundUserTotalAcc; //基金总账户信息
	FundSpConfig m_fund_sp_config;

	int  m_optype;                      // 操作类型
	bool m_need_updateExauAuthLimit; //标记是否需要累加exau限额
	bool m_subAccDrawOk;   // 子账户变更操作
	bool m_balanceChanged;  // 份额存在变化
	
protected:	
	virtual void parseBizInputMsg(char* szMsg) throw (CException) = 0;  //纯虚函数，需自定义业务输入参数
	virtual void packBizReturnMsg(TRPC_SVCINFO* rqst) = 0;  //纯虚函数，需自定义业务输出参数
	
	virtual void CheckParams() throw (CException);
	virtual void CheckFundBind() throw (CException);
	virtual void CheckFundTrade() throw (CException);
	void UpdateTradeState();
	virtual void updateCkvs();
	void updateExauAuthLimitNoExcp();

	// 赎回通知基金公司成功
	void UpdateRedemTradeForSpInfoSuc() throw (CException);
	// 赎回延迟确认:仅通知成功,未确认份额
	virtual void CheckRedemTradeForInfoSuc() throw (CException);
	virtual void BuildRedemTradeForInfoSuc(ST_TRADE_FUND& stRecord) throw (CException);
	virtual void RecordRedemTradeForInfoSuc(ST_TRADE_FUND& stRecord) throw (CException);
	virtual void RecordSpconfigForInfoSuc() throw (CException);
	void UpdateRedemTradeForInfoSuc() throw (CException);

	//赎回成功
	virtual bool CheckRedemTradeForSuc() throw (CException);
	virtual void updateWxPrePayUserBalance()throw (CException);
	virtual void DrawSubacc() throw (CException);
	virtual void checkSpLoaning() throw (CException);
	virtual void BuildRedemTradeForSuc(ST_TRADE_FUND& stRecord) throw (CException);
	virtual void RecordRedemTradeForSuc(ST_TRADE_FUND& stRecord) throw (CException);
	virtual void SyncRedemTradeForSuc(ST_TRADE_FUND& stRecord) throw (CException);
	void UpdateRedemTradeForSuc() throw (CException);

	//赎回确认成功
	virtual bool CheckRedemTradeForAckSuc() throw (CException);
	virtual void BuildRedemTradeForAckSuc(ST_TRADE_FUND& stRecord) throw (CException);
	virtual void RecordRedemTradeForAckSuc(ST_TRADE_FUND& stRecord) throw (CException);
	virtual void SyncRedemTradeForAckSuc(ST_TRADE_FUND& stRecord) throw (CException);
	void UpdateRedemTradeForAckSuc() throw (CException);

	// 赎回确认失败
	virtual bool CheckRedemTradeForAckFail() throw (CException);
	virtual void BuildRedemTradeForAckFail(ST_TRADE_FUND& stRecord) throw (CException);
	virtual void RecordRedemTradeForAckFail(ST_TRADE_FUND& stRecord) throw (CException);
	void UpdateRedemTradeForAckFail() throw (CException);
	
	// 赎回通知超时
	void UpdateRedemTradeForInfoTimeout() throw (CException);
	// 赎回通知超时补单
	void UpdateRedemTradeForBudan() throw (CException);
	
	// 赎回通知失败
	virtual void CheckRedemTradeForInfoFail() throw (CException);
	virtual void BuildRedemTradeForInfoFail(ST_TRADE_FUND& stRecord) throw (CException);
	virtual void RecordRedemTradeForInfoFail(ST_TRADE_FUND& stRecord) throw (CException);
	void UpdateRedemTradeForInfoFail() throw (CException);
	
	// 提现确认赎回完成
	virtual void BuildRedemTradeForFinish(ST_TRADE_FUND& stRecord) throw (CException);
	virtual void RecordRedemTradeForFinish(ST_TRADE_FUND& stRecord) throw (CException);
	void UpdateRedemTradeForFinish() throw (CException);

private:
	// 公共操作
	void doDraw() throw (CException);
	void doDrawReq() throw (CException);
	void doDrawResult(int result) throw (CException);
	void sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy);
	bool payNotifyOvertime(string pay_suc_time);

};

#endif /* _ABSTRACT_REDEM_SP_ACK_H_*/

