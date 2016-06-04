/**
  * FileName: fund_query_balance_order_service.h
  * Author: louisjiang	
  * Version :1.0
  * Date: 2014-05-15
  * Description: 基金交易服务 查询余额流水信息
  */


#ifndef _FUND__QUERY_BALANCE_ORDER_SERVICE_H_
#define _FUND__QUERY_BALANCE_ORDER_SERVICE_H_



class FundQryBalanceOrder
{
public:
    FundQryBalanceOrder(CMySQL* mysql);

    void parseInputMsg(TRPC_SVCINFO* rqst) throw (CException);
    void excute()  throw (CException);
    void packReturnMsg(TRPC_SVCINFO* rqst);


private:

    CParams m_params;                   // 消息参数
    CMySQL* m_fund_conn;                // 基金数据库连接句柄
    ST_BALANCE_ORDER  m_orderInfo;
};

#endif
