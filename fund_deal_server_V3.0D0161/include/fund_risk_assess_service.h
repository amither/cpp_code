/**
  * FileName: fund_nopass_reset_paycard_service.h
  * Author: jiggersong	
  * Version :1.0
  * Date: 2014-01-27
  * Description: 基金交易服务 重置用户的安全卡
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

    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄
    int  risk_type;                      //用户风险承受能力等级
    int agree_risk; //是否匹配
    	FundSpConfig m_fund_sp_config;
};

#endif /* _FUND_RISK_ASSESS_H_*/

