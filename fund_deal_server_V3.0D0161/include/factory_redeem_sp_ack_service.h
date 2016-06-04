/**
  * FileName: factory_redeem_sp_ack_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: 基金申购请求工厂类
  */


#ifndef _FACTORY_REDEEM_SP_ACK_H_
#define _FACTORY_REDEEM_SP_ACK_H_

#include "user_classify.h"
#include "redeem_demand_ack_service.h"
#include "redeem_close_ack_service.h"
#include "redeem_index_ack_service.h"
#include "abstract_redeem_sp_ack_service.h"

class FactoryRedeemSpAck {
	public:
		FactoryRedeemSpAck(CMySQL* mysql);
		~FactoryRedeemSpAck(){}
		void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
		void excute() throw (CException);
		void packReturnMsg(TRPC_SVCINFO* rqst);
		
	private:			
		AbstractRedeemSpAck* redeemSpAck;
		CMySQL* m_pFundCon;                // 基金数据库连接句柄
    
		void CheckToken(CParams& params) throw (CException);
		string GenFundToken(CParams& params);
			
};

#endif /* _FACTORY_REDEEM_SP_ACK_H_*/