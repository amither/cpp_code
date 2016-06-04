/**
  * FileName: redeem_demand_req_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-03-03
  * Description: 活期基金申请申购
  */
  
#include "fund_commfunc.h"
#include "redeem_demand_req_service.h"

RedeemDemandReq::RedeemDemandReq(CMySQL* mysql):AbstractRedeemSpReq(mysql)
{
    TRACE_DEBUG("[RedeemDemandReq] init");
}

void RedeemDemandReq::parseBizInputMsg(char* szMsg) throw (CException)
{
	// 没有特殊的业务参数
	return;
}

/**
  * 检查参数
  */
void RedeemDemandReq::CheckParams() throw (CException)
{
		AbstractRedeemSpReq::CheckParams();
		//默认T+0
    if (m_params.getInt("fetch_type") != DRAW_ARRIVE_TYPE_T1 && m_params.getInt("fetch_type") != DRAW_ARRIVE_TYPE_BA)
    {
        m_params.setParam("fetch_type", DRAW_ARRIVE_TYPE_T0);
    }
}

void RedeemDemandReq::packBizReturnMsg(TRPC_SVCINFO* rqst)
{
	// 没有特殊的业务参数
	return;
}


