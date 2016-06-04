/**
  * FileName: fund_reg_daily_profit_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-8-16
  * Description: 基金交易服务 基金收益登记
  */


#ifndef _FUND_REG_EXP_PROFIT_H_
#define _FUND_REG_EXP_PROFIT_H_

class FundRegExpProfit
{
public:
    FundRegExpProfit(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);

private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);

    void CheckFundBind() throw (CException);
	void AddProfit() throw (CException);

	void QueryFundProfit( bool & isThroughPrecheck) throw (CException);
	void CheckBalance(bool precheck=false)  throw (CException);

	void RecordFundProfit();
	void UpdateProfitInfo();
	void updateCache(FundProfit& fund_profit);
	bool payNotifyOvertime(string pay_suc_time, int inteval);

    void updateUserAccCKV();
	/*
	*获取当天的申购金额，赎回份额
	*/
    void  getTodayTran(ST_TRADE_FUND &trade_fund,LONG &purchase_total_fee,LONG &redem_total_fee);
	/*
	*获取历史未确认的申购金额，赎回份额
	*/
	void  getHistoryNotAckTran(ST_TRADE_FUND &trade_fund, LONG &purchase_total_fee,LONG &redem_total_fee);
	/*
	*获取今日确认的申购份额和赎回份额
	*/
	void  getTodayAckTran(ST_TRADE_FUND &trade_fund, LONG &purchase_total_fee,LONG &redem_total_fee);
	
	/**
	* 计算收益
	*/
	LONG calProfit();
    void UpdatePrecheckInfo();
private:
	TRPC_SVCINFO* m_request;			// 服务请求
    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

    ST_FUND_BIND m_fund_bind;           // 用户账号信息
    FundProfit m_fund_profit;			// 用户收益信息
    FundUserTotalAcc m_fundUserTotalAcc; //基金总账户信息

	int m_curtype;

	bool m_fund_profit_exist;//基金账户收益记录是否存在

    bool m_isThroughPrecheck; //是否已经在检查的时候记录当天的份额信息

    bool m_isWriteLogTimeCost;   //写入只标记，用于控制日志打印频率

    timeval m_tStart;
    timeval m_tEnd;
};

#endif /* _FUND_REG_DAILY_PROFIT_H_ */

