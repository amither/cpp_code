/**
  * FileName: fund_balance_pre_freeze_service.h
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-05-29
  * Description: 份额预冻结
  */

#ifndef _FUND_BALANCE_PRE_FREEZE_SERVICE_H_
#define _FUND_BALANCE_PRE_FREEZE_SERVICE_H_

class FundBalancePreFreeze
{
public:
    FundBalancePreFreeze(CMySQL* mysql);
    ~FundBalancePreFreeze(){}
    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);

private:
    void RecordBalanceFreeze()  throw (CException);
    void CheckBalanceFreeze()  throw (CException);
	void checkFreezeControl()  throw (CException);
    void CheckParams()  throw (CException);
private:
    CParams m_params;                   // 消息参数
    CMySQL* m_pFundCon;                // 基金数据库连接句柄
};




#endif

