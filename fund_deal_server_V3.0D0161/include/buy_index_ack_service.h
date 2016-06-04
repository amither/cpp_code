/**
  * FileName: buy_index_ack_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: 父类：基金申购请求
  */


#ifndef _BUY_INDEX_ACK_H_
#define _BUY_INDEX_ACK_H_

#include "user_classify.h"
#include "abstract_buy_sp_ack_service.h"

class BuyIndexAck:public AbstractBuySpAck
{
	public:
		BuyIndexAck(CMySQL* mysql);
	
	protected:
		void parseBizInputMsg(char* szMsg) throw (CException);
		void packBizReturnMsg(TRPC_SVCINFO* rqst);
		void CheckFundTrade() throw (CException);
		void updateCkvs();
		/**
		  *  支付通知
		  */
		bool CheckParamsForPay() throw (CException);
		void CheckPayTransDate() throw(CException);
		void BuildFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException);
		void RecordFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException);
		double calRedeemRate();
		/**
		  * 份额确认
		  */
		void CheckFundTradeRepeatForSucAck(ST_TRADE_FUND&  stRecord) throw (CException);
		bool CheckFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException);
		void BuildFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException);
		void RecordFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException);
		
	protected:
		FUND_UNCONFIRM m_fundUnconfirm;
		FUND_UNCONFIRM m_update_fundUnconfirm;
		FundTransProcess m_fundIndexTrans;
		FundTransProcess m_update_fundIndexTrans;
};

#define BUY_ACK_UNITS_USEABLE 0 // 申购确认参数:可用份额
#define BUY_ACK_UNITS_CFM_USEABLE 1 // 申购确认参数:已确认份额修改可用
#define BUY_ACK_UNITS_UNUSEABLE 2 //申购确认参数:份额不可用

#endif /* _BUY_INDEX_ACK_H_*/

