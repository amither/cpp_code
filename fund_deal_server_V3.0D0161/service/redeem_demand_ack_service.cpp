/**
  * FileName: redeem_demand_ack_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-03-03
  * Description: 活期基金赎回确认
  */
  
#include "fund_commfunc.h"
#include "redeem_demand_ack_service.h"

RedeemDemandAck::RedeemDemandAck(CMySQL* mysql):AbstractRedeemSpAck(mysql)
{
    TRACE_DEBUG("[RedeemDemandAck] init");
}

void RedeemDemandAck::parseBizInputMsg(char* szMsg) throw (CException)
{
	// 没有特殊的业务参数
	return;
}

/**
  * 检查参数
  */
void RedeemDemandAck::CheckParams() throw (CException)
{
		AbstractRedeemSpAck::CheckParams();
		//默认T+0
    if (m_params.getInt("fetch_type") != DRAW_ARRIVE_TYPE_T1 && m_params.getInt("fetch_type") != DRAW_ARRIVE_TYPE_BA)
    {
        m_params.setParam("fetch_type", DRAW_ARRIVE_TYPE_T0);
    }
}

void RedeemDemandAck::packBizReturnMsg(TRPC_SVCINFO* rqst)
{
	// 没有特殊的业务参数
	return;
}


