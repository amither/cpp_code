/**
  * FileName: fund_update_pay_card_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2014-01-09
  * Description: 基金交易服务 更新用户的支付卡信息
  */


#ifndef _FUND_UPDATE_PAY_CARD_H_
#define _FUND_UPDATE_PAY_CARD_H_

class FundUpdatePayCard
{
public:
    FundUpdatePayCard(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);

	void UpdatePayCard() ;
	
private:

    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄
    
};

#endif /* _FUND_SPACCOUNT_FREEZE_H_*/

