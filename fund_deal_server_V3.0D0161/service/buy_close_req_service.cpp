/**
  * FileName: buy_close_req_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-03-03
  * Description: ���ڻ��������깺
  */
  
#include "fund_commfunc.h"
#include "buy_close_req_service.h"

BuyCloseReq::BuyCloseReq(CMySQL* mysql):AbstractBuySpReq(mysql)
{
    TRACE_DEBUG("[BuyCloseReq] init");
}

void BuyCloseReq::parseBizInputMsg(char* szMsg) throw (CException)
{
	// û�������ҵ�����
	return;
}

/**
  * �������ʲ�
  */
void BuyCloseReq::CheckFundSpLimit() throw (CException)
{	
    AbstractBuySpReq::CheckFundSpLimit();
	
	/* ����Ƿ���Թ���*/
	checkPermissionBuyCloseFund(m_fund_bind.Ftrade_id, m_fund_sp_config, m_params.getString("systime"), true);
}

void BuyCloseReq::packBizReturnMsg(TRPC_SVCINFO* rqst)
{
	// û�������ҵ�����
	return;
}


