/**
  * FileName: abstract_redeem_sp_req_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-03-09
  * Description: �����׷��� �����������
  */


#ifndef _ABSTRACT_REDEEM_SP_REQ_H_
#define _ABSTRACT_REDEEM_SP_REQ_H_

class AbstractRedeemSpReq
{
public:
    AbstractRedeemSpReq(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst,char* szMsg) throw (CException);
    virtual void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);
    void setSpConfig(FundSpConfig fundSpConfig);

protected:
		virtual void parseBizInputMsg(char* szMsg) throw (CException) = 0;  //���麯�������Զ���ҵ���������
		virtual void packBizReturnMsg(TRPC_SVCINFO* rqst) = 0;  //���麯�������Զ���ҵ���������
	
    virtual void CheckParams() throw (CException);

    virtual void CheckFundBind() throw (CException);
    virtual void CheckFundBindSpAcc() throw (CException);
    virtual void CheckFundBalance();
    virtual void checkSpLoaning() throw (CException);
    virtual void CheckFundTrade() throw (CException);
    virtual void CheckAuthLimit() throw (CException);
	virtual void CheckRedeem();
	
    virtual void RecordFundTrade()  throw (CException);
	virtual void BuildFundTrade()  throw (CException);
	void UpdateFundTrade() throw (CException);
	void doDrawReq()  throw (CException);

    virtual bool IsForceRedem(string fetch_no);
	virtual void updateCkvs();

	
protected:

	TRPC_SVCINFO* m_request;			// ��������
    CParams m_params;                   // ��Ϣ����
    string m_spid;                      // �̻�SPID
    CMySQL* m_pFundCon;                // �������ݿ����Ӿ��

    ST_FUND_BIND m_fund_bind;           // �û��˺���Ϣ
    FundBindSp m_fund_bind_sp_acc;	
    FundSpConfig m_fund_sp_config;
    ST_TRADE_FUND m_stTradeRedem; 	// ���׼�¼
    ST_UNFREEZE_FUND m_stUnFreezedata; //���ᵥ��¼
    
    bool m_redemTradeExist;

private:
	bool payNotifyOvertime(string pay_suc_time);

};

#endif /* _ABSTRACT_REDEEM_SP_REQ_H_*/