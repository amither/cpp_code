/**
  * FileName: abstract_buy_sp_ack_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: ���ࣺ�����깺����
  */


#ifndef _ABSTRACT_BUY_SP_ACK_H_
#define _ABSTRACT_BUY_SP_ACK_H_

#include "user_classify.h"

class AbstractBuySpAck
{
	public:
		AbstractBuySpAck(CMySQL* mysql);
		virtual ~AbstractBuySpAck(){}
		void parseInputMsg(TRPC_SVCINFO* rqst,char* szMsg) throw (CException);
		virtual void excute() throw (CException);
		void packReturnMsg(TRPC_SVCINFO* rqst);
		void setSpConfig(FundSpConfig fundSpConfig);
		
	protected:
		TRPC_SVCINFO* m_request;			// ��������
		CParams m_params;                   // ��Ϣ����
		CMySQL* m_pFundCon;                // �������ݿ����Ӿ��
	  
		ST_FUND_BIND m_fund_bind;           // �û��˺���Ϣ
		FundBindSp m_fund_bind_sp_acc;	
		ST_TRADE_FUND m_stTradeBuy; // �����¼
		FundSpConfig m_fund_sp_config; //����������Ϣ
		TStr2StrMap m_bindCareInfo ;
		ST_FUND_CONTROL_INFO m_controlInfo; // Ԥ�����ܿ���Ϣ		
		ST_FREEZE_FUND m_freeze_fund; // ���������Ϣ

		int  m_optype;                      // ��������
		bool  need_refund;                      // ��Ҫ�˿�
		int refund_reason;
		bool  m_fund_bind_exist;
		string refund_desc; //�˿�ԭ������
		string m_acc_time; 
		bool m_doSaveOnly; // ֻ�������˻�
		bool m_bBuyTotalAdded; // �ж��Ƿ��ۼ�spconfig
	
	protected:		
		virtual void parseBizInputMsg(char* szMsg) throw (CException) = 0;  //���麯�������Զ���ҵ���������
		virtual void packBizReturnMsg(TRPC_SVCINFO* rqst) = 0;  //���麯�������Զ���ҵ���������

		virtual void parseBizInputMsgComm(char* szMsg) throw (CException); //������͸������
		virtual void CheckParams() throw (CException);
		virtual void CheckFundTrade() throw (CException);
		virtual void CheckFundBind() throw (CException);
		virtual void CheckFundBindSpAcc() throw (CException);
		void UpdateTradeState();
		virtual void updateCkvs();
		void doSave() throw(CException);
		// �����깺ȷ��
		void UpdateFundTradeForReq() throw (CException);
		// ֧��֪ͨ
		virtual bool CheckPayRepeat() throw (CException);
		virtual bool CheckBalancePay() throw (CException);
		virtual void RecordFundBindPay() throw (CException);
		virtual void checkSamePayCard();
		virtual void checkUserTotalShare() throw (CException);	
		virtual void checkFreezePrepayCard() throw (CException);	
		virtual void CheckPayTransDate() throw(CException);
		virtual bool CheckParamsForPay() throw (CException);
		virtual bool CheckFundTradeForPay() throw (CException);
		virtual void checkSpconfigBuyOverFull(const string& tradeDate);
		virtual void BuildFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException);
		virtual void updateFundFreezeBill(const string& fund_vdate ) throw(CException);
		virtual void RecordFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException);
		virtual void SyncFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException);
		virtual void sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy);
		virtual void updateSpconfigTotalBuy(const string& strStatDay,bool RefunAllowed);
		virtual double calRedeemRate();
		void UpdateFundTradeForPay() throw (CException);

		// ֧��֪ͨ�̻�ȷ��
		virtual void BuildFundTradeForPayAck(ST_TRADE_FUND&  stRecord) throw (CException);
		virtual void CheckFundTradeForPayAck() throw (CException);
		virtual void UpdateFundTradeForCoupon() throw (CException);
		void UpdateFundTradeForPayAck() throw (CException);
		
		// �깺ȷ���̻�ȷ��
		virtual bool CheckFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException);
		virtual void BuildFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException);
		virtual void RecordFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException);
		void UpdateFundTradeForSucAck() throw (CException);

		void updateExauAuthLimitNoExcp();
	private:
		void checkBindserialno()throw (CException);
		bool payNotifyOvertime(string pay_suc_time);
		
};

#endif /* _ABSTRACT_BUY_SP_ACK_H_*/

