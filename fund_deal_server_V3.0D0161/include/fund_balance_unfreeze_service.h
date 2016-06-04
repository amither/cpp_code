/**
  * FileName: fund_balance_unfreeze_service.h
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-05-29
  * Description: 份额解冻
  */

#ifndef _FUND_BALANCE_UNFREEZE_SERVICE_H_
#define _FUND_BALANCE_UNFREEZE_SERVICE_H_

class FundBalanceUnFreeze
{
public:
    FundBalanceUnFreeze(CMySQL* mysql);
    ~FundBalanceUnFreeze(){}
    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);

private:
    void CheckParams()  throw (CException);
    void CheckBalanceUnFreeze(bool mustExist)  throw (CException);
    void CheckFreezeRecord()  throw (CException);
    void RecordBalanceUnFreeze()  throw (CException);
    bool unfreezeSubAccBalance()  throw (CException);
    void updateBalanceFreeze()  throw (CException);
    void checkToken() throw (CException);
    //更新解冻单状态
    void updateBalanceUnFreeze()  throw (CException);
    void preRecordUnfreezeBill()throw (CException);
    void CheckFundBind()  throw (CException);
    
private:
    CParams m_params;                   // 消息参数
    CMySQL* m_pFundCon;                // 基金数据库连接句柄
    ST_FREEZE_FUND  m_freezeBill;
    ST_UNFREEZE_FUND m_unfreezeData;
    bool m_bFreezeSunaccSuc;
    int m_subAccErrCode;
    string m_sunAccErrInfo;
    ST_FUND_BIND m_fund_bind;
};


#endif



