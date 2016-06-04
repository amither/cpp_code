/**
  * FileName: buy_index_req_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-03-03
  * Description: ETF基金申请申购
  */


#ifndef _BUY_INDEX_REQ_H_
#define _BUY_INDEX_REQ_H_

#include "user_classify.h"
#include "abstract_buy_sp_req_service.h"


class BuyIndexReq:public AbstractBuySpReq
{
	public:
		BuyIndexReq(CMySQL* mysql);
	protected:
		
		void parseBizInputMsg(char* szMsg) throw (CException);
		void packBizReturnMsg(TRPC_SVCINFO* rqst);
		// 重载:检查指数型基金特性
		void CheckParams() throw (CException);
		void CheckFundSpLimit() throw (CException);

	private:
		void CheckETFBuyDayLimit(int uid ,const string &sys_time,LONG total_fee, FundSpConfig& data) throw (CException);
};

#endif /* _BUY_INDEX_REQ_H_*/