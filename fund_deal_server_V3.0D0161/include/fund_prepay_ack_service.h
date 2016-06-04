/**
  * FileName: fund_wx_pay_ack_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-11-07
  * Description: 基金交易服务 非实名认证用户(暂无法开通理财账户)预支付确认接口
  */


#ifndef _FUND_PREPAY_ACK_H_
#define _FUND_PREPAY_ACK_H_

class FundPrepayAck
{
public:
    FundPrepayAck(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);

	void CheckFundPrepay() throw (CException);
	void UpdateFundPrepay();

	void UpdatePrepayForAuthen();
	void UpdatePrepayForPayOk();
	void UpdatePrepayForRefund();

	
private:

    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

	FundPrepay m_fund_prepay;		// 预支付记录
	ST_TRADE_FUND m_stTradeBuy; // 购买记录
    
};

#endif /* _FUND_WX_PAY_ACK_H_ */

