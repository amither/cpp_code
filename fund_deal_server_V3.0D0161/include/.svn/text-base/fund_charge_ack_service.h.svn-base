/**
  * FileName: fund_charge_ack_service.h
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-07-23
  * Description: 理财通余额充值确认
  */

#ifndef _FUND_CHARGE_ACK_SERVICE_H_
#define _FUND_CHARGE_ACK_SERVICE_H_

class FundChargeAck
{
public:
    FundChargeAck(CMySQL* mysql);
    ~FundChargeAck(){};
    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);
private:

    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
    void CheckFundBind() throw (CException);
    void CheckChargeOrder(bool LockQuery)throw (CException);
    void UpdateChargeOrder()throw (CException);
    void doSubaccCharge()throw (CException);
    void RecordChargeOrder()throw (CException);
    void RecordRelationOrder()throw (CException);
    void PreRecordChargeOrder()  throw (CException);
    void UpdateFundBalanceConfig()throw (CException);
    void checkSafePayCard()  throw (CException);
    void queryFundTransAccTime()throw (CException);
    void UpdateChargeOrderNoRefundFlag()  throw (CException);
    void checkUserTotalAsset()throw (CException);
    void UpdateUserAssetLevel() throw (CException);
        
    CParams m_params;                   // 消息参数
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

    ST_FUND_BIND m_fund_bind;           // 用户开户信息 
    ST_BALANCE_ORDER m_charge_data;
    bool m_bChargeSubaccSuc;
    int m_subAccErrCode;
    string m_subAccErrInfo;
    TRPC_SVCINFO*  m_request;
    bool m_bNeedRefund;
    string m_refundDesc;
    FundBalanceConfig m_fundBaCfg;
    string m_fund_trans_acc_time;
};



#endif
