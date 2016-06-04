/**
  * FileName: FundUpdateBind.h
  * Author: jessiegao	
  * Version :1.0
  */


#ifndef _FUND_UPDATE_BIND_H_
#define _FUND_UPDATE_BIND_H_

class FundUpdateBind
{
public:
    FundUpdateBind(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);

private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
    void updateFundBind() throw (CException);
	void CheckFundBind() throw (CException);
	void updateCKV() throw (CException);


private:
    TRPC_SVCINFO* m_request;			// 服务请求
    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄
    ST_FUND_BIND m_fund_bind;
};

#endif 

