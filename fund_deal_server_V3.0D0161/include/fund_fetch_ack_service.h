/**
  * FileName: fund_fetch_ack_service.h
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-07-23
  * Description: ���ͨ�������ȷ��
  */

#ifndef _FUND_FETCH_ACK_SERVICE_H_
#define _FUND_FETCH_ACK_SERVICE_H_

class FundFetchAck
{
public:
    FundFetchAck(CMySQL* mysql);
    ~FundFetchAck(){};
    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);
private:

    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
    void CheckFundBind() throw (CException);
    void CheckFetchOrder()throw (CException);
    void UpdateFetchOrder()throw (CException);
    void doSubaccFetchResult()throw (CException);
    void CheckFundTrade()  throw (CException);
    void CheckOrderState() throw (CException);
    void checkTotalFetchNum()  throw (CException);
    
    CParams m_params;                   // ��Ϣ����
    CMySQL* m_pFundCon;                // �������ݿ����Ӿ��

    ST_FUND_BIND m_fund_bind;           // �û�������Ϣ 
    ST_BALANCE_ORDER m_fetch_data;

    TRPC_SVCINFO*  m_request;
    string m_refund_desc;
};



#endif




