/**
  * FileName: fund_reg_close_profit_service.h
  * Author: wenlonwang	
  * Version :1.0
  */


#ifndef _FUND_REG_CLOSE_PROFIT_H_
#define _FUND_REG_CLOSE_PROFIT_H_

class FundRegCloseProfit
{
public:
    FundRegCloseProfit(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);

private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
	void parseCloseFundDetail() throw (CException);

    void CheckFundBind() throw (CException);
	void AddProfit() throw (CException);

	void QueryFundProfit() throw (CException);
	void CheckBalance()  throw (CException);

	void RecordFundProfit();
	void UpdateProfitInfo();
	void updateCache(FundProfit& fund_profit);
	void CheckProfitRecord() throw (CException);
	void addEndProfitToTrans(const char* tailRedemId, LONG tailProfit);
	void updateFundCloseProfit();
	bool payNotifyOvertime(string pay_suc_time, int inteval);
	LONG calBalanceFeeLastProfit(const FundCloseTrans &closeTrans);
	LONG calBalanceFeeInBuy(const FundCloseTrans &closeTrans);

	void doSave() throw(CException);
	
	void saveCloseProfitRecord() throw(CException);

private:
	TRPC_SVCINFO* m_request;			// 服务请求
    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄

    ST_FUND_BIND m_fund_bind;           // 用户账号信息
    FundProfit m_fund_profit;			// 用户收益信息
    FundUserTotalAcc m_fundUserTotalAcc; //基金总账户信息
    vector<FundCloseTrans> m_fundCloseTransVec; //定期交易记录
    map<string, FundCloseTransProfit> m_fundCloseTransProfitMap; //输入的收益明细, key为封闭结束时间
    map<string, FundCloseProfitRecord> m_fundCloseProfitMap; //输入的收益明细, key为封闭结束时间

    LONG m_tail_profit; //扫尾收益，扫尾收益对账
	int m_curtype;
	bool m_fund_profit_exist;
	bool m_is_final_profit; // 判断是否是所有期次的最后一期期末收益

};

#endif 

