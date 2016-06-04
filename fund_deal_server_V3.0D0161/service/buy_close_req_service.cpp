/**
  * FileName: buy_close_req_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-03-03
  * Description: 活期基金申请申购
  */
  
#include "fund_commfunc.h"
#include "buy_close_req_service.h"

BuyCloseReq::BuyCloseReq(CMySQL* mysql):AbstractBuySpReq(mysql)
{
    TRACE_DEBUG("[BuyCloseReq] init");
}

void BuyCloseReq::parseBizInputMsg(char* szMsg) throw (CException)
{
	// 没有特殊的业务参数
	return;
}

/**
  * 检查基金资产
  */
void BuyCloseReq::CheckFundSpLimit() throw (CException)
{	
    AbstractBuySpReq::CheckFundSpLimit();
	
	/* 检查是否可以购买*/
	checkPermissionBuyCloseFund(m_fund_bind.Ftrade_id, m_fund_sp_config, m_params.getString("systime"), true);
}

void BuyCloseReq::packBizReturnMsg(TRPC_SVCINFO* rqst)
{
	// 没有特殊的业务参数
	return;
}


