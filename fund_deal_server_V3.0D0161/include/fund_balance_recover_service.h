/**
  * FileName: fund_balance_recover_service.h
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-07-23
  * Description: ���ͨ���ز�
  */

#ifndef _FUND_BALANCE_RECOVER_SERVICE_H_
#define _FUND_BALANCE_RECOVER_SERVICE_H_

class FundBalanceRecover
{
public:
    FundBalanceRecover(CMySQL* mysql);
    ~FundBalanceRecover(){};
    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);
private:

    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
    void CheckRollList()throw (CException);
    void RecordRollList()throw (CException);
    void UpdateFundBalanceConfig()throw (CException);
    void checkFundBalanceConfigTradeDate() throw (CException);
    void UpdateRollList()throw (CException);

    
    CParams m_params;                   // ��Ϣ����
    CMySQL* m_pFundCon;                // �������ݿ����Ӿ��

    FundBalanceConfig m_fundBaCfg;
    LONG m_sup_backer_fee;
    ST_BALANCE_RECOVER m_recoverData;
};



#endif


