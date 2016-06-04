/**
  * FileName: fund_close_end_redem_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2014-7-30
  * Description: ���ɶ�����ĩ��ؼ�¼
  */


#ifndef _FUND_DEAL_CLOSE_END_REDEM_H_
#define _FUND_DEAL_CLOSE_END_REDEM_H_

class FundCloseEndRedem
{
public:
    FundCloseEndRedem(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
	void CheckCloseTrans() throw (CException);
	void checkReq() throw (CException);
	void checkReqRepeat() throw (CException);
	void checkAck() throw (CException);
	void checkAckRepeat() throw (CException);
    void CheckFundBindSpAcc() throw (CException);
    void CheckFundBalance() throw (CException);
	void checkAccTime() throw (CException);
    void RecordFundTrade() throw (CException);
	void updateToRedemSuccess() throw (CException);
	void GenerFundTrade() throw (CException);
	void updateCkvs();
	void updateExauAuthLimitNoExcp();
	void doDraw() throw (CException);
	void RecordRedemTradeForSuc() throw (CException);
	void sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy);
	void recordSpLoaning() throw (CException);

	
private:
	TRPC_SVCINFO* m_request;			// ��������
    CParams m_params;                   // ��Ϣ����
    string m_spid;                      // �̻�SPID
    CMySQL* m_pFundCon;                // �������ݿ����Ӿ��

	FundCloseTrans m_close_trans;   // ���ڽ��ױ�����
	
    ST_FUND_BIND m_fund_bind;           // �û��˺���Ϣ
    FundBindSp m_fund_bind_sp_acc;	
    ST_TRADE_FUND  m_stRecord;  // ���׼�¼
    FundUserTotalAcc m_fundUserTotalAcc; //�������˻���Ϣ

    bool m_RedemTradeExist;
	LONG m_real_fee;	//����
	LONG m_total_fee;	//����+����
	int m_optype;

};

#endif

