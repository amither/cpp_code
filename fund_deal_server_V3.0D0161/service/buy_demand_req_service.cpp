/**
  * FileName: buy_demand_req_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-03-03
  * Description: ���ڻ��������깺
  */
  
#include "fund_commfunc.h"
#include "buy_demand_req_service.h"

BuyDemandReq::BuyDemandReq(CMySQL* mysql):AbstractBuySpReq(mysql)
{
    TRACE_DEBUG("[BuyDemandReq] init");
}

void BuyDemandReq::parseBizInputMsg(char* szMsg) throw (CException)
{
	// û�������ҵ�����
	return;
}

void BuyDemandReq::packBizReturnMsg(TRPC_SVCINFO* rqst)
{
	// û�������ҵ�����
	return;
}


