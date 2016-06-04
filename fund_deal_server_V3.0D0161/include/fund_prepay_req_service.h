/**
  * FileName: fund_prepay_req_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-11-7
  * Description: 基金交易服务 非实名认证用户(暂无法开通理财账户)预支付请求接口
  */


#ifndef _FUND_PREPAY_REQ_H_
#define _FUND_PREPAY_REQ_H_

class FundPrepayReq
{
public:
    FundPrepayReq(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:

    void CheckParams() throw (CException);

	void CheckFundPrepay() throw (CException);
	void RecordFundPrepay();
	void RecordFundTrade();

	
private:

    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

    FundPrepay m_fund_prepay;		// 预支付记录

	bool m_fund_prepay_exist;
    
};

#endif 

