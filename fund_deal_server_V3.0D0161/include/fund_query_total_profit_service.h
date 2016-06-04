/**
  * FileName: fund_query_total_profit_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-11-05
  * Description: 基金交易服务 查询总收益及昨日收益
  */


#ifndef _FUND_QUERY_TOTAL_PROFIT_SERVICE_H_
#define _FUND_QUERY_TOTAL_PROFIT_SERVICE_H_
class FundQueryTotalProfit
{
public:
    FundQueryTotalProfit(CMySQL* mysql,int type=NO_CHECK_LOGIN);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:

	void CheckParams() throw (CException);
	void CheckFundBind() throw (CException);

    void queryTotalProfit() throw (CException);

private:

    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_fund_conn;                // 基金数据库连接句柄

    FundProfit m_fund_profit;           // 用户总收益信息
    vector<FundProfit> m_fundProfitVec; //各基金收益信息

    int m_servicetype;


};

#endif 

