/**
  * FileName: buy_close_req_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-03-03
  * Description: 定期基金申请申购
  */


#ifndef _FUND_DEAL_BUY_CLOSE_REQ_H_
#define _FUND_DEAL_BUY_CLOSE_REQ_H_

#include "user_classify.h"
#include "abstract_buy_sp_req_service.h"


class BuyCloseReq:public AbstractBuySpReq
{
	public:		
		BuyCloseReq(CMySQL* mysql);
		
	protected:
		
		void parseBizInputMsg(char* szMsg) throw (CException);
		void CheckFundSpLimit() throw (CException);
		void packBizReturnMsg(TRPC_SVCINFO* rqst);
};

#endif /* _FUND_DEAL_BUY_CLOSE_REQ_H_*/