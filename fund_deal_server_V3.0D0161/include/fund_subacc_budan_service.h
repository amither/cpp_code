/**
  * FileName: fund_subacc_budan_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2014-01-27
  * Description: 基金交易服务 子账户补单
  */


#ifndef _FUND_SUBACC_BUDAN_H_
#define _FUND_SUBACC_BUDAN_H_

class FundSubaccBudan
{
public:
    FundSubaccBudan(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);;

private:

    void CheckParams() throw (CException);
	void CheckFundBind() throw (CException);
    void CheckFundTrade() throw (CException);
	void CheckProfitRecord() throw (CException);
	void SubaccBudan() throw (CException);
	void doSave() throw(CException);

private:
	TRPC_SVCINFO* m_request;			// 服务请求
    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄
    
    ST_FUND_BIND m_fund_bind;           // 用户账号信息
    ST_TRADE_FUND m_stTradeBuy; // 购买记录

	int  m_optype;                      // 操作类型
};

#endif /* _FUND_DEAL_BUY_SP_ACK_H_*/

