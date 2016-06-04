/**
  * FileName: redem_close_req_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-03-03
  * Description: 活期基金申请申购
  */


#ifndef _REDEEM_CLOSE_REQ_H_
#define _REDEEM_CLOSE_REQ_H_

#include "user_classify.h"
#include "abstract_redeem_sp_req_service.h"


class RedeemCloseReq:public AbstractRedeemSpReq
{
	public:		
		RedeemCloseReq(CMySQL* mysql);
		
	protected:
		void parseBizInputMsg(char* szMsg) throw (CException);
		void packBizReturnMsg(TRPC_SVCINFO* rqst);
		void CheckParams() throw (CException);
		void CheckFundTrade() throw (CException);
		void CheckFundTradeRepeat() throw (CException);
		void BuildFundTrade()  throw (CException);
	protected:
		FundCloseTrans m_close_trans;
};

#endif /* _REDEEM_CLOSE_REQ_H_*/