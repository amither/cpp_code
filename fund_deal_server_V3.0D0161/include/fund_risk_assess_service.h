/**
  * FileName: fund_nopass_reset_paycard_service.h
  * Author: jiggersong	
  * Version :1.0
  * Date: 2014-01-27
  * Description: �����׷��� �����û��İ�ȫ��
  */


#ifndef _FUND_RISK_ASSESS_H_
#define _FUND_RISK_ASSESS_H_

class  FundRiskAssess
{
public:
    FundRiskAssess(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);

	void UpdateRiskAssess() ;
	void insertFundUserRiskLog();
	void CheckSpAgreeRisk();
	void CheckUserAssessRisk(int risk_score);
	
private:

    CParams m_params;                   // ��Ϣ����
    string m_spid;                      // �̻�SPID
    CMySQL* m_pFundCon;                // �������ݿ����Ӿ��
    int  risk_type;                      //�û����ճ��������ȼ�
    int agree_risk; //�Ƿ�ƥ��
    	FundSpConfig m_fund_sp_config;
};

#endif /* _FUND_RISK_ASSESS_H_*/

