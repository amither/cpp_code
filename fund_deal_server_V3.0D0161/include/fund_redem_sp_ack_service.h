/**
  * FileName: fund_redem_sp_ack_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-8-19
  * Description: �����׷��� �������ȷ��
  */


#ifndef _FUND_DEAL_REDEM_SP_ACK_H_
#define _FUND_DEAL_REDEM_SP_ACK_H_

class FundRedemSpAck
{
public:
    FundRedemSpAck(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);;

private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);

	void CheckFundBind() throw (CException);

    void CheckFundTrade() throw (CException);
    void CheckTradeTime() throw (CException);
    void UpdateTradeState();
	void UpdateRedemTradeForSuc() throw (CException);
	void UpdateRedemTradeForTimeout() throw (CException);
	void UpdateRedemTradeForFail() throw (CException);
	void UpdateRedemTradeForFinish() throw (CException);
	void UpdateRedemTradeForBudan() throw (CException);
	void checkSpLoaning() throw (CException);
	void doDraw() throw (CException);
	void sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy);
	void updateExauAuthLimitNoExcp();
       void updateWxPrePayUserBalance()throw (CException);

private:
	TRPC_SVCINFO* m_request;			// ��������

    CParams m_params;                   // ��Ϣ����
    string m_spid;                      // �̻�SPID
    CMySQL* m_pFundCon;                // �������ݿ����Ӿ��

    ST_TRADE_FUND m_stTradeBuy; // ���׼�¼
    ST_FUND_BIND m_fund_bind;           // �û��˺���Ϣ
    FundUserTotalAcc m_fundUserTotalAcc; //�������˻���Ϣ

	int  m_optype;                      // ��������
	int m_draw_arrive_type; //��������
	int m_loading_type;//�Ƿ��ߵ����˻�
	bool m_stop_fetch;
	bool m_need_updateExauAuthLimit; //����Ƿ���Ҫ�ۼ�exau�޶�
	string m_subAccControlList; //��Լ���ⶳ���� ���˻��ܿص���
	bool m_subAccDrawOk;

};

#endif /* _FUND_DEAL_REDEM_SP_ACK_H_*/

