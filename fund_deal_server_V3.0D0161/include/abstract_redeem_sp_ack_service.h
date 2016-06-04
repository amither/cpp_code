/**
  * FileName: abstract_redeem_sp_ack_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-3-12
  * Description: �����׷��� �������ȷ��
  */


#ifndef _ABSTRACT_REDEM_SP_ACK_H_
#define _ABSTRACT_REDEM_SP_ACK_H_

class AbstractRedeemSpAck
{
public:
    AbstractRedeemSpAck(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst,char* szMsg) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);
    void setSpConfig(FundSpConfig fundSpConfig);

protected:
	TRPC_SVCINFO* m_request;			// ��������

	CParams m_params;                   // ��Ϣ����
	CMySQL* m_pFundCon;                // �������ݿ����Ӿ��

	ST_TRADE_FUND m_stTradeBuy; // ���׼�¼
	ST_FUND_BIND m_fund_bind;           // �û��˺���Ϣ
	FundUserTotalAcc m_fundUserTotalAcc; //�������˻���Ϣ
	FundSpConfig m_fund_sp_config;

	int  m_optype;                      // ��������
	bool m_need_updateExauAuthLimit; //����Ƿ���Ҫ�ۼ�exau�޶�
	bool m_subAccDrawOk;   // ���˻��������
	bool m_balanceChanged;  // �ݶ���ڱ仯
	
protected:	
	virtual void parseBizInputMsg(char* szMsg) throw (CException) = 0;  //���麯�������Զ���ҵ���������
	virtual void packBizReturnMsg(TRPC_SVCINFO* rqst) = 0;  //���麯�������Զ���ҵ���������
	
	virtual void CheckParams() throw (CException);
	virtual void CheckFundBind() throw (CException);
	virtual void CheckFundTrade() throw (CException);
	void UpdateTradeState();
	virtual void updateCkvs();
	void updateExauAuthLimitNoExcp();

	// ���֪ͨ����˾�ɹ�
	void UpdateRedemTradeForSpInfoSuc() throw (CException);
	// ����ӳ�ȷ��:��֪ͨ�ɹ�,δȷ�Ϸݶ�
	virtual void CheckRedemTradeForInfoSuc() throw (CException);
	virtual void BuildRedemTradeForInfoSuc(ST_TRADE_FUND& stRecord) throw (CException);
	virtual void RecordRedemTradeForInfoSuc(ST_TRADE_FUND& stRecord) throw (CException);
	virtual void RecordSpconfigForInfoSuc() throw (CException);
	void UpdateRedemTradeForInfoSuc() throw (CException);

	//��سɹ�
	virtual bool CheckRedemTradeForSuc() throw (CException);
	virtual void updateWxPrePayUserBalance()throw (CException);
	virtual void DrawSubacc() throw (CException);
	virtual void checkSpLoaning() throw (CException);
	virtual void BuildRedemTradeForSuc(ST_TRADE_FUND& stRecord) throw (CException);
	virtual void RecordRedemTradeForSuc(ST_TRADE_FUND& stRecord) throw (CException);
	virtual void SyncRedemTradeForSuc(ST_TRADE_FUND& stRecord) throw (CException);
	void UpdateRedemTradeForSuc() throw (CException);

	//���ȷ�ϳɹ�
	virtual bool CheckRedemTradeForAckSuc() throw (CException);
	virtual void BuildRedemTradeForAckSuc(ST_TRADE_FUND& stRecord) throw (CException);
	virtual void RecordRedemTradeForAckSuc(ST_TRADE_FUND& stRecord) throw (CException);
	virtual void SyncRedemTradeForAckSuc(ST_TRADE_FUND& stRecord) throw (CException);
	void UpdateRedemTradeForAckSuc() throw (CException);

	// ���ȷ��ʧ��
	virtual bool CheckRedemTradeForAckFail() throw (CException);
	virtual void BuildRedemTradeForAckFail(ST_TRADE_FUND& stRecord) throw (CException);
	virtual void RecordRedemTradeForAckFail(ST_TRADE_FUND& stRecord) throw (CException);
	void UpdateRedemTradeForAckFail() throw (CException);
	
	// ���֪ͨ��ʱ
	void UpdateRedemTradeForInfoTimeout() throw (CException);
	// ���֪ͨ��ʱ����
	void UpdateRedemTradeForBudan() throw (CException);
	
	// ���֪ͨʧ��
	virtual void CheckRedemTradeForInfoFail() throw (CException);
	virtual void BuildRedemTradeForInfoFail(ST_TRADE_FUND& stRecord) throw (CException);
	virtual void RecordRedemTradeForInfoFail(ST_TRADE_FUND& stRecord) throw (CException);
	void UpdateRedemTradeForInfoFail() throw (CException);
	
	// ����ȷ��������
	virtual void BuildRedemTradeForFinish(ST_TRADE_FUND& stRecord) throw (CException);
	virtual void RecordRedemTradeForFinish(ST_TRADE_FUND& stRecord) throw (CException);
	void UpdateRedemTradeForFinish() throw (CException);

private:
	// ��������
	void doDraw() throw (CException);
	void doDrawReq() throw (CException);
	void doDrawResult(int result) throw (CException);
	void sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy);
	bool payNotifyOvertime(string pay_suc_time);

};

#endif /* _ABSTRACT_REDEM_SP_ACK_H_*/

