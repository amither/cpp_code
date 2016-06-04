/**
  * FileName: fund_balance_freeze_service.h
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-05-29
  * Description: �ݶ��
  */

#ifndef _FUND_BALANCE_FREEZE_SERVICE_H_
#define _FUND_BALANCE_FREEZE_SERVICE_H_

class FundBalanceFreeze
{
public:
    FundBalanceFreeze(CMySQL* mysql);
    ~FundBalanceFreeze(){}
    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);

private:
    void CheckParams()  throw (CException);
    void CheckBalanceFreeze()  throw (CException);
    void CheckFundBind() throw (CException);
    void CheckFundBindSpAcc() throw (CException);
    void updateBalanceFreeze()throw (CException);
    void CheckBuyRecord()throw (CException);
    void freezeSubAccBalance() throw (CException);
    void checkToken() throw (CException);
    
private:
    CParams m_params;                   // ��Ϣ����
    CMySQL* m_pFundCon;                // �������ݿ����Ӿ��
    FundSpConfig m_FundSpConfig;
    ST_FREEZE_FUND  m_freezeBill;

    ST_FUND_BIND m_fund_bind;           // �û�������Ϣ
};


#endif

