/**
  * FileName: fund_fetch_req_service.h
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-07-23
  * Description: 理财通余额提现请求
  */

#ifndef _FUND_FETCH_REQ_SERVICE_H_
#define _FUND_FETCH_REQ_SERVICE_H_

class FundFetchReq
{
public:
    FundFetchReq(CMySQL* mysql);
    ~FundFetchReq(){};
    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);
private:

    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
    void CheckFundBind() throw (CException);
    void CheckFetchOrder(bool needLockQuery)throw (CException);
    void RecordFetchOrder()throw (CException);
    void RecordRelationOrder()throw (CException);
    void UpdateFetchOrderSuc()throw (CException);
    void UpdateFetchOrderRefund()throw (CException);
    void doSubaccFetch()throw (CException);
    void doSubaccFetchCancle()throw (CException);

    void checkTotalFetchNum()  throw (CException);

    void CheckFetchExauLimit()throw (CException);

    void PreCheckFetchBalance()throw (CException);

    bool queryFetchOrder() throw (CException);

    void UpdateFetchExauLimit();

    void UpdateFetchOrderBackerFlag()throw (CException);

	void updateCKV() throw (CException);
    
    CParams m_params;                   // 消息参数
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

    ST_FUND_BIND m_fund_bind;           // 用户开户信息 
    ST_BALANCE_ORDER m_fetch_data;
    int m_subAccErrCode;
    string m_subAccErrInfo;
    bool m_bOrderExist;
    bool m_bNeedUnFreeze;
    string m_unFreezeDesc;
    int m_unFreeze_Errcode;
    bool m_bNeedBackerFee;
    FundBalanceConfig m_fundBaCfg;
};



#endif

