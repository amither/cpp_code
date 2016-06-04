/**
  * FileName: abstract_buy_sp_ack_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: 父类：基金申购请求
  */


#ifndef _ABSTRACT_BUY_SP_ACK_H_
#define _ABSTRACT_BUY_SP_ACK_H_

#include "user_classify.h"

class AbstractBuySpAck
{
	public:
		AbstractBuySpAck(CMySQL* mysql);
		virtual ~AbstractBuySpAck(){}
		void parseInputMsg(TRPC_SVCINFO* rqst,char* szMsg) throw (CException);
		virtual void excute() throw (CException);
		void packReturnMsg(TRPC_SVCINFO* rqst);
		void setSpConfig(FundSpConfig fundSpConfig);
		
	protected:
		TRPC_SVCINFO* m_request;			// 服务请求
		CParams m_params;                   // 消息参数
		CMySQL* m_pFundCon;                // 基金数据库连接句柄
	  
		ST_FUND_BIND m_fund_bind;           // 用户账号信息
		FundBindSp m_fund_bind_sp_acc;	
		ST_TRADE_FUND m_stTradeBuy; // 购买记录
		FundSpConfig m_fund_sp_config; //基金属性信息
		TStr2StrMap m_bindCareInfo ;
		ST_FUND_CONTROL_INFO m_controlInfo; // 预付卡受控信息		
		ST_FREEZE_FUND m_freeze_fund; // 冻结基金信息

		int  m_optype;                      // 操作类型
		bool  need_refund;                      // 需要退款
		int refund_reason;
		bool  m_fund_bind_exist;
		string refund_desc; //退款原因描述
		string m_acc_time; 
		bool m_doSaveOnly; // 只更新子账户
		bool m_bBuyTotalAdded; // 判断是否累加spconfig
	
	protected:		
		virtual void parseBizInputMsg(char* szMsg) throw (CException) = 0;  //纯虚函数，需自定义业务输入参数
		virtual void packBizReturnMsg(TRPC_SVCINFO* rqst) = 0;  //纯虚函数，需自定义业务输出参数

		virtual void parseBizInputMsgComm(char* szMsg) throw (CException); //公共的透传参数
		virtual void CheckParams() throw (CException);
		virtual void CheckFundTrade() throw (CException);
		virtual void CheckFundBind() throw (CException);
		virtual void CheckFundBindSpAcc() throw (CException);
		void UpdateTradeState();
		virtual void updateCkvs();
		void doSave() throw(CException);
		// 申请申购确认
		void UpdateFundTradeForReq() throw (CException);
		// 支付通知
		virtual bool CheckPayRepeat() throw (CException);
		virtual bool CheckBalancePay() throw (CException);
		virtual void RecordFundBindPay() throw (CException);
		virtual void checkSamePayCard();
		virtual void checkUserTotalShare() throw (CException);	
		virtual void checkFreezePrepayCard() throw (CException);	
		virtual void CheckPayTransDate() throw(CException);
		virtual bool CheckParamsForPay() throw (CException);
		virtual bool CheckFundTradeForPay() throw (CException);
		virtual void checkSpconfigBuyOverFull(const string& tradeDate);
		virtual void BuildFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException);
		virtual void updateFundFreezeBill(const string& fund_vdate ) throw(CException);
		virtual void RecordFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException);
		virtual void SyncFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException);
		virtual void sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy);
		virtual void updateSpconfigTotalBuy(const string& strStatDay,bool RefunAllowed);
		virtual double calRedeemRate();
		void UpdateFundTradeForPay() throw (CException);

		// 支付通知商户确认
		virtual void BuildFundTradeForPayAck(ST_TRADE_FUND&  stRecord) throw (CException);
		virtual void CheckFundTradeForPayAck() throw (CException);
		virtual void UpdateFundTradeForCoupon() throw (CException);
		void UpdateFundTradeForPayAck() throw (CException);
		
		// 申购确认商户确认
		virtual bool CheckFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException);
		virtual void BuildFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException);
		virtual void RecordFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException);
		void UpdateFundTradeForSucAck() throw (CException);

		void updateExauAuthLimitNoExcp();
	private:
		void checkBindserialno()throw (CException);
		bool payNotifyOvertime(string pay_suc_time);
		
};

#endif /* _ABSTRACT_BUY_SP_ACK_H_*/

