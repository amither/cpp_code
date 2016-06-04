/**
  * FileName: fund_reg_profit_rate_service.h
  * Author: wenlonwang	
  * Version :1.0
  * Date: 2013-12-02
  * Description: 基金交易服务 登记基金收益率
  */


#ifndef _FUND_REG_PROFIT_RATE_H_
#define _FUND_REG_PROFIT_RATE_H_

class FundRegProfitRate
{
public:
    FundRegProfitRate(CMySQL* mysql,int para);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);

private:
    string GenFundToken();
    void CheckToken() throw (CException); 
    void CheckParams() throw (CException);
	
	void checkProfitRate() throw (CException);

	void regProfitRate();

	void checkFundSpConfig();

	void updateProfitRateCache();

private:
	TRPC_SVCINFO* m_request;			// 服务请求
    CParams m_params;                   // 消息参数
    string m_spid;                      // 商户SPID
    CMySQL* m_pFundCon;                // 基金数据库连接句柄
    int m_optype;                //操作类型:0:收益入账;1:导入外部明细收益率
    FundSpConfig m_fundSpConfig;
	FundProfitRate  m_fundProfitRate;

	/**
        *   定义明细收益率:
             一月至一年的区间收益率、涨跌幅、跟踪误差以及年化收益
            
        *   定义基本收益率
            货币基金的七日年化、D-1万分收益率
            指数基金的日涨幅、日净值
        */
    bool m_updateDetailRate;        //是否更新明细收益率
    bool m_updateBaseRate;        //是否需要更新基础收益率

};
	
#define OP_TYPE_REG_RATE_FROM_PARTNER 0 // 来自基金公司的数据更新收益率
#define OP_TYPE_REG_RATE_FROM_OUTER  1 // 来自外部数据更新收益率

#endif 

