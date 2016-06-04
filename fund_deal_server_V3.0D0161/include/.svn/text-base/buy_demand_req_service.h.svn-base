/**
  * FileName: buy_demand_req_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-03-03
  * Description: 活期基金申请申购
  */


#ifndef _FUND_DEAL_BUY_DEMAND_REQ_H_
#define _FUND_DEAL_BUY_DEMAND_REQ_H_

#include "user_classify.h"
#include "abstract_buy_sp_req_service.h"


class BuyDemandReq:public AbstractBuySpReq
{
	public:		
		BuyDemandReq(CMySQL* mysql);
		
	protected:
		void parseBizInputMsg(char* szMsg) throw (CException);
		void packBizReturnMsg(TRPC_SVCINFO* rqst);
};

#endif /* _FUND_DEAL_BUY_DEMAND_REQ_H_*/