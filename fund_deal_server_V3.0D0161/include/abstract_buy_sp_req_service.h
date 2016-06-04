/**
  * FileName: abstract_buy_sp_req_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: ���ࣺ�����깺����
  */


#ifndef _ABSTRACT_BUY_SP_REQ_H_
#define _ABSTRACT_BUY_SP_REQ_H_

#include "user_classify.h"

class AbstractBuySpReq
{
	public:
		AbstractBuySpReq(CMySQL* mysql);
		virtual ~AbstractBuySpReq(){}
		void parseInputMsg(TRPC_SVCINFO* rqst,char* szMsg) throw (CException);
		virtual void excute() throw (CException);
		void packReturnMsg(TRPC_SVCINFO* rqst);
		void setSpConfig(FundSpConfig fundSpConfig);
		
	protected:
		CParams m_params;      // ��Ϣ����
		CMySQL* m_pFundCon;    // ���ݿ����Ӿ��
		int m_payChannel;	   //pay_channel(0�����п�   1�����ͨ��� 2: ���� )
		string m_channelId;    
		ST_FUND_BIND m_fund_bind; 
		FundBindSp m_fund_bind_sp_acc;
		ST_TRADE_FUND m_stTradeBuy;
		FundSpConfig m_fund_sp_config;
		
		bool m_bRepeatEntry;
		bool m_bBuyTradeExist;
		
		virtual void parseBizInputMsg(char* szMsg) throw (CException) = 0;  //���麯�������Զ���ҵ���������
		virtual void packBizReturnMsg(TRPC_SVCINFO* rqst) = 0;  //���麯�������Զ���ҵ���������
		
		virtual void CheckParams() throw (CException);  //  ������
		virtual void CheckFundBind() throw (CException);  //  ��鲢��ѯ�û�
		virtual void CheckFundBindSpAcc() throw (CException);  //  ��鲢��ѯ�û�����
		virtual void CheckUserTotalShare() throw (CException); //  ������ʲ�
		virtual void CheckFundTrade() throw (CException); //  ��鲢��ѯ���׵�
		virtual void CheckFundSpLimit() throw (CException); //  �������̻�����
		virtual void RecordFundTrade(); //  ��¼���׵�
		virtual void CheckUserTempFail() throw (CException); //  ����û�������ʱʧ�����

		//���������ֶ��е�channel_id������������channel_id��pay_channel(0�����п�   1�����ͨ��� 2: ���� )
		//�磬 0_68|fm_3_unknown,  channel_idΪ68|fm_3_unknown, pay_channelΪ0
		virtual void CheckChannelId() throw (CException);
    
	private:
		void UpdateFundTradeForReq();  //  ���½��׵�
};

#endif /* _ABSTRACT_BUY_SP_REQ_H_*/

