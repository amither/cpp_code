/**
  * FileName: fund_transfer_req_service.h
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-05-16
  * Description: 份额转换请求接口
  */

#ifndef _FUND_DEAL_TRANSFER_REQ_H_
#define _FUND_DEAL_TRANSFER_REQ_H_

class FundTransferReq
{
public:
    FundTransferReq(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
    void CheckFundBind() throw (CException);
    void CheckFundBindSpAcc() throw (CException);
    void checkUserBalance() throw (CException);
    void CheckFundTransfer() throw (CException);
    void RecordFundTransfer()throw (CException);
    void checkTransferTimes() throw (CException);
    void checkSpRedemRate() throw (CException);
    
private:
    CParams m_params;                   // 消息参数
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

    ST_FUND_BIND m_fund_bind;           // 用户开户信息
    FundBindSp m_fund_bind_orisp_acc;	
    FundBindSp m_fund_bind_newsp_acc;	
    ST_TRANSFER_FUND m_transferOrder; // 转换单
    FundSpConfig m_fund_orisp_config;
    FundSpConfig m_fund_newsp_config;

};


#endif

