/**
  * FileName: fund_query_balance_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-12-31
  * Description: 基金交易服务 查询余额增值账户余额
  */

#ifndef _FUND_QUERY_BALANCE_SERVICE_H_
#define _FUND_QUERY_BALANCE_SERVICE_H_
class FundQueryBalance
{
public:
    FundQueryBalance(CMySQL* mysql,int type=NO_CHECK_LOGIN);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:

	void CheckParams() throw (CException);
	void CheckFundBind() throw (CException);
	void queryBalance();

private:

    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_fund_conn;                // 基金数据库连接句柄
    int m_servicetype;
    vector<SubaccUser> m_subaccListUser;
    map<int,string> m_cutypeToSpid;
    vector<int> m_subacc_curtype_list;

};

#endif 

