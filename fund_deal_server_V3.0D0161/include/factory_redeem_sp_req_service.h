/**
  * FileName: factory_redeem_sp_req_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: �����깺���󹤳���
  */


#ifndef _FACTORY_REDEEM_SP_REQ_H_
#define _FACTORY_REDEEM_SP_REQ_H_

#include "user_classify.h"
#include "redeem_demand_req_service.h"
#include "redeem_close_req_service.h"
#include "redeem_index_req_service.h"
#include "abstract_redeem_sp_req_service.h"

class FactoryRedeemSpReq {
	public:
		FactoryRedeemSpReq(CMySQL* mysql);
		~FactoryRedeemSpReq(){}
		void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
		void excute() throw (CException);
		void packReturnMsg(TRPC_SVCINFO* rqst);
		
	private:			
		AbstractRedeemSpReq* redeemSpReq;
		CMySQL* m_pFundCon;                // �������ݿ����Ӿ��
    
		void CheckToken(CParams& params) throw (CException);
		string GenFundToken(CParams& params);
			
};

#endif /* _FACTORY_REDEEM_SP_REQ_H_*/