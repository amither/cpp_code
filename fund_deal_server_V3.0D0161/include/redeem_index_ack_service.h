/**
  * FileName: redeem_index_ack_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-03-03
  * Description: 活期基金申请申购
  */


#ifndef _REDEEM_INDEX_ACK_H_
#define _REDEEM_INDEX_ACK_H_

#include "user_classify.h"
#include "abstract_redeem_sp_ack_service.h"


class RedeemIndexAck:public AbstractRedeemSpAck
{
	public:		
		RedeemIndexAck(CMySQL* mysql);
		
	protected:
		void parseBizInputMsg(char* szMsg) throw (CException);
		void packBizReturnMsg(TRPC_SVCINFO* rqst);
		void CheckFundTrade() throw (CException);
		bool CheckRedemTradeForAckSuc() throw (CException);
        void BuildRedemTradeForAckSuc(ST_TRADE_FUND& stRecord) throw (CException);
		void RecordRedemTradeForAckSuc(ST_TRADE_FUND& stRecord) throw (CException);
		void BuildRedemTradeForInfoFail(ST_TRADE_FUND& stRecord) throw (CException);
		void RecordRedemTradeForInfoFail(ST_TRADE_FUND& stRecord) throw (CException);
		void BuildRedemTradeForAckFail(ST_TRADE_FUND& stRecord) throw (CException);
		void RecordRedemTradeForAckFail(ST_TRADE_FUND& stRecord) throw (CException);		
		void BuildRedemTradeForFinish(ST_TRADE_FUND& stRecord) throw (CException);
		void RecordRedemTradeForFinish(ST_TRADE_FUND& stRecord) throw (CException);
		void updateCkvs();

	private:		
		FundTransProcess m_fundIndexTrans;
		FundTransProcess m_update_fundIndexTrans;
};

#endif /* _REDEEM_INDEX_ACK_H_*/
