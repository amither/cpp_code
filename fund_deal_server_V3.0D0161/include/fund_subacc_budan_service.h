/**
  * FileName: fund_subacc_budan_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2014-01-27
  * Description: �����׷��� ���˻�����
  */


#ifndef _FUND_SUBACC_BUDAN_H_
#define _FUND_SUBACC_BUDAN_H_

class FundSubaccBudan
{
public:
    FundSubaccBudan(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);;

private:

    void CheckParams() throw (CException);
	void CheckFundBind() throw (CException);
    void CheckFundTrade() throw (CException);
	void CheckProfitRecord() throw (CException);
	void SubaccBudan() throw (CException);
	void doSave() throw(CException);

private:
	TRPC_SVCINFO* m_request;			// ��������
    CParams m_params;                   // ��Ϣ����
    string m_spid;                      // �̻�SPID
    CMySQL* m_pFundCon;                // �������ݿ����Ӿ��
    
    ST_FUND_BIND m_fund_bind;           // �û��˺���Ϣ
    ST_TRADE_FUND m_stTradeBuy; // �����¼

	int  m_optype;                      // ��������
};

#endif /* _FUND_DEAL_BUY_SP_ACK_H_*/

