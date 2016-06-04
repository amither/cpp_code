/**
  * FileName: factory_buy_sp_req_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: 基金申购请求工厂类
  */


#ifndef _FACTORY_BUY_SP_REQ_H_
#define _FACTORY_BUY_SP_REQ_H_

#include "user_classify.h"
#include "buy_demand_req_service.h"
#include "buy_close_req_service.h"
#include "buy_index_req_service.h"
#include "abstract_buy_sp_req_service.h"

class FactoryBuySpReq {
	public:
		FactoryBuySpReq(CMySQL* mysql);
		~FactoryBuySpReq(){}
		void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
		void excute() throw (CException);
		void packReturnMsg(TRPC_SVCINFO* rqst);
		
	private:			
		AbstractBuySpReq* buySpReq;
		CMySQL* m_pFundCon;                // 基金数据库连接句柄
    
		void CheckToken(CParams& params) throw (CException);
		string GenFundToken(CParams& params);
			
};

#endif /* _FACTORY_BUY_SP_REQ_H_*/

