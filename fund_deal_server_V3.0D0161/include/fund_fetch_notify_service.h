/**
  * FileName: fund_fetch_notify_service.h
  * Version :1.0
  * Date: 2015-03-02
  * Description: 理财通余额提现冻结通知确认
  */

#ifndef _FUND_FETCH_NOTIFY_SERVICE_H_
#define _FUND_FETCH_NOTIFY_SERVICE_H_

class FundFetchNotify
{
public:
    FundFetchNotify(CMySQL* mysql);
    FundFetchNotify(){};
    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);
private:

    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
    void CheckFundBind() throw (CException);
    void CheckFetchOrder()throw (CException);
    void UpdateFetchOrder()throw (CException);
    void CheckFundTrade()  throw (CException);
    void CheckOrderState() throw (CException);
    
    CParams m_params;                   // 消息参数
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

    ST_FUND_BIND m_fund_bind;           // 用户开户信息 
    ST_BALANCE_ORDER m_fetch_data;

    TRPC_SVCINFO*  m_request;
};



#endif




