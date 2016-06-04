/**
  * FileName: fund_wx_pay_ack_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-11-07
  * Description: �����׷��� ��ʵ����֤�û�(���޷���ͨ����˻�)Ԥ֧��ȷ�Ͻӿ�
  */


#ifndef _FUND_PREPAY_ACK_H_
#define _FUND_PREPAY_ACK_H_

class FundPrepayAck
{
public:
    FundPrepayAck(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);

	void CheckFundPrepay() throw (CException);
	void UpdateFundPrepay();

	void UpdatePrepayForAuthen();
	void UpdatePrepayForPayOk();
	void UpdatePrepayForRefund();

	
private:

    CParams m_params;                   // ��Ϣ����
    string m_spid;                      // �̻�SPID
    CMySQL* m_pFundCon;                // �������ݿ����Ӿ��

	FundPrepay m_fund_prepay;		// Ԥ֧����¼
	ST_TRADE_FUND m_stTradeBuy; // �����¼
    
};

#endif /* _FUND_WX_PAY_ACK_H_ */

