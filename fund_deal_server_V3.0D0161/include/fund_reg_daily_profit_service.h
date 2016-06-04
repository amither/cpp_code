/**
  * FileName: fund_reg_daily_profit_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-8-16
  * Description: �����׷��� ��������Ǽ�
  */


#ifndef _FUND_REG_DAILY_PROFIT_H_
#define _FUND_REG_DAILY_PROFIT_H_

class FundRegProfit
{
public:
    FundRegProfit(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);

private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);

    void CheckFundBind() throw (CException);
	void AddProfit() throw (CException);

	void QueryFundProfit( bool & isThroughPrecheck) throw (CException);
	void CheckBalance(bool precheck=false)  throw (CException);

	void RecordFundProfit();
	void UpdateProfitInfo();
	void updateCache(FundProfit& fund_profit);
	void CheckProfitRecord() throw (CException);
	bool payNotifyOvertime(string pay_suc_time, int inteval);

	void doSave() throw(CException);
	void updateUserAccCKV();
    
    void UpdatePrecheckInfo();
private:
	TRPC_SVCINFO* m_request;			// ��������
    CParams m_params;                   // ��Ϣ����
    string m_spid;                      // �̻�SPID
    CMySQL* m_pFundCon;                // �������ݿ����Ӿ��

    ST_FUND_BIND m_fund_bind;           // �û��˺���Ϣ
    FundProfit m_fund_profit;			// �û�������Ϣ
    FundUserTotalAcc m_fundUserTotalAcc; //�������˻���Ϣ

	int m_curtype;

	bool m_fund_profit_exist;

    bool m_isThroughPrecheck;

    bool m_isWriteLogTimeCost;

    timeval m_tStart;
    timeval m_tEnd;
};

#endif /* _FUND_REG_DAILY_PROFIT_H_ */

