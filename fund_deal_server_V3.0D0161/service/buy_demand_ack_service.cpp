/**
  * FileName: abstract_buy_sp_ack_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: 父类：基金申购请求
  */


#include "fund_commfunc.h"
#include "buy_demand_ack_service.h"

BuyDemandAck::BuyDemandAck(CMySQL* mysql):AbstractBuySpAck(mysql)
{	
    TRACE_DEBUG("[BuyDemandAck] init");
}

/**
  * service step 1: 解析输入参数
  */
void BuyDemandAck::parseBizInputMsg(char* szMsg)  throw (CException)
{
    // 没有特殊的业务参数
    return;
}
bool BuyDemandAck::CheckParamsForPay() throw (CException)
{
	bool result = AbstractBuySpAck::CheckParamsForPay();
	// 兼容原来没有传入fund_units参数
	// 活期类型需在支付通知的时候记录份额
	if(m_params.getLong("fund_units")==0)
	{
		m_params.setParam("fund_units",m_params.getLong("total_fee"));
	}
	return result;
}

/**
* 打包输出参数
*/
void BuyDemandAck::packBizReturnMsg(TRPC_SVCINFO* rqst)
{
	// 没有特殊的业务参数
	return;
}


