/**
  * FileName: fundacc_bind_sp_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-8-16
  * Description: 基金交易服务 基金开户头文件
  */


#ifndef _FUND_DEAL_BIND_SPACC_H_
#define _FUND_DEAL_BIND_SPACC_H_

class FundBindSpAcc
{
public:
    FundBindSpAcc(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);

private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
    void CheckAuthParam();
    void CheckFundBind() throw (CException);
	void CheckFundBindSpAcc() throw (CException);
	bool ExistMasterSpAcc(); 
	void recoveryUserBindsp(ST_FUND_UNBIND &unbindInfo) throw (CException);
    void DoRegProcess() throw (CException);
    void DoAuthenRegAndPreBindSpAcc() throw (CException);
    void DoBindSpAck();

    void AddFundBind();
    void AddFundBindSpAcc();
    void UpdateFundBindSpAcc(int acct_type);
	void UpdateBindSpStateAcctime();
	void create_user();
	void CheckAssessRisk();
	void UpdateRiskAssess();

private:

	TRPC_SVCINFO* m_request;			// 服务请求
    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_fund_conn;                // 基金数据库连接句柄

    ST_FUND_BIND m_fund_bind;           // 用户账号信息
    FundBindSp m_fund_bind_sp_acc;		// 基金用户绑定基金公司交易账户信息
    FundSpConfig m_fund_sp_config;
    ST_USER_RISK user_risk;
    bool m_bind_exist;                  // 账号信息是否存在
    bool m_bind_spacc_exist;				// 绑定基金增值帐号是否存在

    int  m_optype;                      // 操作类型
    string m_uin;                       // 用户qq号
    bool m_is_recovery;
    bool m_user_assess_exist; //评测流水存在
};

#endif /* _FUND_DEAL_BIND_SPACC_H_ */

