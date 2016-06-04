/**
  * FileName: fund_query_bind_sp_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-9-19
  * Description: 基金交易服务 查询余额增值账户绑定信息
  */


#ifndef _FUND_QUERY_BIND_SP_H_
#define _FUND_QUERY_BIND_SP_H_

class FundQueryBindSp
{
public:
    FundQueryBindSp(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);

private:

    void CheckParams() throw (CException);

    void CheckFundBind() throw (CException);
	void CheckFundBindSpAcc() throw (CException);


private:

    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_fund_conn;                // 基金数据库连接句柄

    ST_FUND_BIND m_fund_bind;           // 用户账号信息
    FundBindSp m_fund_bind_sp_acc;		// 基金用户绑定基金公司交易账户信息

    bool m_bind_exist;                  // 账号信息是否存在
    bool m_bind_spacc_exist;				// 绑定基金增值帐号是否存在

    int  relation_exist;                      // 绑定关系是否存在 0:不存在  1:存在
};

#endif /* _FUND_QUERY_BIND_SP_H_ */

