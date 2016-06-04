/**
  * FileName: redeem_demand_ack_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-03-03
  * Description: 活期基金申请申购
  */


#ifndef _REDEEM_DEMAND_ACK_H_
#define _REDEEM_DEMAND_ACK_H_

#include "user_classify.h"
#include "abstract_redeem_sp_ack_service.h"


class RedeemDemandAck:public AbstractRedeemSpAck
{
	public:		
		RedeemDemandAck(CMySQL* mysql);
		
	protected:
		void parseBizInputMsg(char* szMsg) throw (CException);
		void packBizReturnMsg(TRPC_SVCINFO* rqst);
		void CheckParams() throw (CException);
};

#endif /* _REDEEM_DEMAND_ACK_H_*/