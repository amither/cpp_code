/**
  * FileName: abstract_buy_sp_ack_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: 父类：基金申购请求
  */


#ifndef _BUY_DEMAND_ACK_H_
#define _BUY_DEMAND_ACK_H_

#include "user_classify.h"
#include "abstract_buy_sp_ack_service.h"

class BuyDemandAck:public AbstractBuySpAck
{
	public:
		BuyDemandAck(CMySQL* mysql);
	
	protected:
		void parseBizInputMsg(char* szMsg) throw (CException);

		/** 支付成功重载 
		 */
		bool CheckParamsForPay() throw (CException);
		void packBizReturnMsg(TRPC_SVCINFO* rqst);
};

#endif /* _ABSTRACT_BUY_SP_ACK_H_*/

