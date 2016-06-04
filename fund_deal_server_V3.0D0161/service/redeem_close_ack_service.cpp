/**
  * FileName: redeem_close_ack_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-03-03
  * Description: 活期基金赎回确认
  */
  
#include "fund_commfunc.h"
#include "redeem_close_ack_service.h"

RedeemCloseAck::RedeemCloseAck(CMySQL* mysql):AbstractRedeemSpAck(mysql)
{
    TRACE_DEBUG("[RedeemCloseAck] init");
}

void RedeemCloseAck::parseBizInputMsg(char* szMsg) throw (CException)
{
	// 没有特殊的业务参数
	return;
}

/**
  * 检查参数
  */
void RedeemCloseAck::CheckParams() throw (CException)
{
	AbstractRedeemSpAck::CheckParams();
}

void RedeemCloseAck::packBizReturnMsg(TRPC_SVCINFO* rqst)
{
	// 没有特殊的业务参数
	return;
}


