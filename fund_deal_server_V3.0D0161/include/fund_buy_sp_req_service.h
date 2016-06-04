/**
  * FileName: fund_buy_sp_req_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-8-16
  * Description: 基金交易服务 基金申购请求
  */


#ifndef _FUND_DEAL_BUY_SP_REQ_H_
#define _FUND_DEAL_BUY_SP_REQ_H_

#include "user_classify.h"


class FundBuySpReq
{
public:
    FundBuySpReq(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);

    void CheckFundBind() throw (CException);
	void CheckFundBindSpAcc() throw (CException);

    void CheckFundTrade() throw (CException);
    void CheckTradeTime() throw (CException);
    void RecordFundTrade();
	void UpdateFundTradeForReq();
	void checkUserTotalShare() throw (CException);
	void checkUserTempFail() throw (CException);
	void CheckChannelId() throw (CException);
private:

    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄
    CRpcWrapper* m_user_rpc;            // user_info 连接句柄

    ST_FUND_BIND m_fund_bind;           // 用户账号信息
    FundBindSp m_fund_bind_sp_acc;	
	ST_TRADE_FUND m_stTradeBuy; // 购买记录
	FundSpConfig m_fund_sp_config;

	bool m_bRepeatEntry;
    bool m_bBuyTradeExist;

    int m_payChannel;	   //pay_channel(0：银行卡   1：理财通余额 2: 网银 )
	string m_channelId; 

};

#endif /* _FUND_DEAL_BUY_SP_REQ_H_*/

