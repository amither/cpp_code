/**
  * FileName: fund_charge_req_service.h
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-07-23
  * Description: 理财通余额充值请求
  */

#ifndef _FUND_CHARGE_REQ_SERVICE_H_
#define _FUND_CHARGE_REQ_SERVICE_H_

class FundChargeReq
{
public:
    FundChargeReq(CMySQL* mysql);
    ~FundChargeReq(){};
    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);
private:

    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
    void CheckFundBind() throw (CException);
    void CheckChargeOrder()throw (CException);
    void RecordChargeOrder()throw (CException);
    void RecordRelationOrder()throw (CException);
    void checkUserTotalAsset()throw (CException);
    
    CParams m_params;                   // 消息参数
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

    ST_FUND_BIND m_fund_bind;           // 用户开户信息 
    FundBalanceConfig m_fundBaCfg;
};



#endif
