/**
  * FileName: fund_buy_sp_ack_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-8-16
  * Description: 基金交易服务 基金申购确认
  */


#ifndef _FUND_DEAL_BUY_SP_ACK_H_
#define _FUND_DEAL_BUY_SP_ACK_H_
#include "user_classify.h"


class FundBuySpAck
{
public:
    FundBuySpAck(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);;

private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
	void CheckFundBind() throw (CException);
	void CheckFundBindSpAcc() throw (CException);

    void CheckFundTrade() throw (CException);
    //void CheckTradeTime() throw (CException);
    void UpdateTradeState();
	void UpdateFundTradeForReq() throw (CException);
	void UpdateFundTradeForPay() throw (CException);
	void UpdateFundTradeForAck() throw (CException);
	void UpdateFundTradeForCoupon() throw (CException);
	void updateChangeSp();
	void checkAndUpdateChangeSp();
	void doSave() throw(CException);
	void recordRewardToProfit();
	void checkUserTotalShare() throw (CException);
	void checkSamePayCard();
	void ChangeMasterTradeAcc();
	void ChangeMasterTradeAcc(string new_spid, string old_spid);
	void UpdateDefaultTradeAcc();
	void checkFreezePrepayCard() throw (CException);
	void queryFundSpAndFundcodeInfo();
	void checkUserPermissionBuyCloseFund();
	void recordCloseFund(ST_TRADE_FUND& stRecord);
	void checkCloseEndDate();

	void sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy);
      void ckeckBindserialno()throw (CException);
      void updateCloseDayTotalBuy(ST_TRADE_FUND& stRecord,bool RefunAllowed);
      void checkCloseDayTotalBuyOverFull(ST_TRADE_FUND& stRecord);
      void updateFundFreezeBill(const string& fund_vdate ) throw(CException);

private:
	TRPC_SVCINFO* m_request;			// 服务请求
    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄
    
    ST_FUND_BIND m_fund_bind;           // 用户账号信息
    FundBindSp m_fund_bind_sp_acc;	
    ST_TRADE_FUND m_stTradeBuy; // 购买记录
    FundUserTotalAcc m_fundUserTotalAcc; //基金总账户信息
    FundCloseTrans m_fundCloseTrans; //定期产品交易记录
    FundCloseCycle m_fundCloseCycle; // 定期交易周期
    FundSpConfig m_fundSpConfig;//基金属性信息

	int  m_optype;                      // 操作类型
	bool  need_refund;                      // 需要退款
	bool  pay_card_notequal;                // 再次支付的卡和之前支付的卡不一致
	bool  m_fund_bind_exist;
	string refund_desc; //退款原因描述
	bool need_updateKVFundBind ; //
	int m_close_fund_seqno; //定期产品可用序列
   string m_acc_time; 
   TStr2StrMap m_bindCareInfo ;
   bool  m_bCloseBuyTotalAdded;
	ST_FUND_CONTROL_INFO m_controlInfo; // 预付卡受控信息		
	ST_FREEZE_FUND m_freeze_fund; // 冻结基金信息
	int refund_reason;
};

#endif /* _FUND_DEAL_BUY_SP_ACK_H_*/

