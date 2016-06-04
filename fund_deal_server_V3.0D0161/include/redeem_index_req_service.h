/**
  * FileName: redeem_index_req_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-03-03
  * Description: ���ڻ��������깺
  */


#ifndef _REDEEM_INDEX_REQ_H_
#define _REDEEM_INDEX_REQ_H_

#include "user_classify.h"
#include "abstract_redeem_sp_req_service.h"


class RedeemIndexReq:public AbstractRedeemSpReq
{
	public:		
		RedeemIndexReq(CMySQL* mysql);
		
	protected:
		void parseBizInputMsg(char* szMsg) throw (CException);
		void packBizReturnMsg(TRPC_SVCINFO* rqst);
		void CheckParams() throw (CException);
		void BuildFundTrade()  throw (CException);
		void RecordFundTrade()  throw (CException);
		void updateCkvs();
		
	private:		
		FundTransProcess m_fundIndexTrans;
};

#endif /* _REDEEM_INDEX_REQ_H_*/