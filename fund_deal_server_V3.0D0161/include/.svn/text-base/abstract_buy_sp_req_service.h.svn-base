/**
  * FileName: abstract_buy_sp_req_service.h
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: 父类：基金申购请求
  */


#ifndef _ABSTRACT_BUY_SP_REQ_H_
#define _ABSTRACT_BUY_SP_REQ_H_

#include "user_classify.h"

class AbstractBuySpReq
{
	public:
		AbstractBuySpReq(CMySQL* mysql);
		virtual ~AbstractBuySpReq(){}
		void parseInputMsg(TRPC_SVCINFO* rqst,char* szMsg) throw (CException);
		virtual void excute() throw (CException);
		void packReturnMsg(TRPC_SVCINFO* rqst);
		void setSpConfig(FundSpConfig fundSpConfig);
		
	protected:
		CParams m_params;      // 消息参数
		CMySQL* m_pFundCon;    // 数据库连接句柄
		int m_payChannel;	   //pay_channel(0：银行卡   1：理财通余额 2: 网银 )
		string m_channelId;    
		ST_FUND_BIND m_fund_bind; 
		FundBindSp m_fund_bind_sp_acc;
		ST_TRADE_FUND m_stTradeBuy;
		FundSpConfig m_fund_sp_config;
		
		bool m_bRepeatEntry;
		bool m_bBuyTradeExist;
		
		virtual void parseBizInputMsg(char* szMsg) throw (CException) = 0;  //纯虚函数，需自定义业务输入参数
		virtual void packBizReturnMsg(TRPC_SVCINFO* rqst) = 0;  //纯虚函数，需自定义业务输出参数
		
		virtual void CheckParams() throw (CException);  //  检查参数
		virtual void CheckFundBind() throw (CException);  //  检查并查询用户
		virtual void CheckFundBindSpAcc() throw (CException);  //  检查并查询用户开户
		virtual void CheckUserTotalShare() throw (CException); //  检查总资产
		virtual void CheckFundTrade() throw (CException); //  检查并查询交易单
		virtual void CheckFundSpLimit() throw (CException); //  检查基金商户限制
		virtual void RecordFundTrade(); //  记录交易单
		virtual void CheckUserTempFail() throw (CException); //  检查用户开户临时失败情况

		//解析请求字段中的channel_id，生成真正的channel_id和pay_channel(0：银行卡   1：理财通余额 2: 网银 )
		//如， 0_68|fm_3_unknown,  channel_id为68|fm_3_unknown, pay_channel为0
		virtual void CheckChannelId() throw (CException);
    
	private:
		void UpdateFundTradeForReq();  //  更新交易单
};

#endif /* _ABSTRACT_BUY_SP_REQ_H_*/

