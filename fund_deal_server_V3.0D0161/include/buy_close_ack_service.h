/**
  * FileName: buy_close_ack_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: 父类：基金申购请求
  */


#ifndef _BUY_CLOSE_ACK_H_
#define _BUY_CLOSE_ACK_H_

#include "user_classify.h"
#include "abstract_buy_sp_ack_service.h"

class BuyCloseAck:public AbstractBuySpAck
{
	public:
		BuyCloseAck(CMySQL* mysql);
		
	protected:
		
		FundCloseTrans m_fundCloseTrans; //定期产品交易记录
		FundCloseCycle m_fundCloseCycle; // 定期交易周期
		int m_close_fund_seqno; //定期产品可用序列
		bool  m_bCloseBuyTotalAdded;
		
	protected:
		void parseBizInputMsg(char* szMsg) throw (CException);
		void packBizReturnMsg(TRPC_SVCINFO* rqst);

		void CheckParams() throw (CException);	
		void updateCkvs();

		/** 支付成功重载 
		 */
		bool CheckParamsForPay() throw (CException);
		bool CheckPayRepeat() throw (CException);
		bool CheckClosePayRepeat() throw (CException);
		void CheckPayTransDate() throw(CException);
		bool CheckFundTradeForPay() throw (CException);
		void BuildFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException);	

		/** 支付通知商户重载 
		 */
		void CheckFundTradeForPayAck() throw (CException);
		
	private:
		void checkUserPermissionBuyCloseFund();
    
};

#endif /* _BUY_CLOSE_ACK_H_*/

