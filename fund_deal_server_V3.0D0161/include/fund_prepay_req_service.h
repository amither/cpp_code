/**
  * FileName: fund_prepay_req_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-11-7
  * Description: �����׷��� ��ʵ����֤�û�(���޷���ͨ����˻�)Ԥ֧������ӿ�
  */


#ifndef _FUND_PREPAY_REQ_H_
#define _FUND_PREPAY_REQ_H_

class FundPrepayReq
{
public:
    FundPrepayReq(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:

    void CheckParams() throw (CException);

	void CheckFundPrepay() throw (CException);
	void RecordFundPrepay();
	void RecordFundTrade();

	
private:

    CParams m_params;                   // ��Ϣ����
    string m_spid;                      // �̻�SPID
    CMySQL* m_pFundCon;                // �������ݿ����Ӿ��

    FundPrepay m_fund_prepay;		// Ԥ֧����¼

	bool m_fund_prepay_exist;
    
};

#endif 

