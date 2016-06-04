/**
  * FileName: fund_query_balance_order_service.cpp
  * Author: louisjiang	
  * Version :1.0
  * Date: 2014-05-15
  * Description: 基金交易服务 查询余额流水信息
  */

#include "fund_commfunc.h"
#include "fund_query_balance_order_service.h"

FundQryBalanceOrder::FundQryBalanceOrder(CMySQL* mysql)
{
    m_fund_conn = mysql; 
    memset(&m_orderInfo,0,sizeof(m_orderInfo));
}


/**
  * service step 1: 解析输入参数
  */
void FundQryBalanceOrder::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char *pMsg = (char*)(rqst->idata);
    // 读取参数
    m_params.readStrParam(pMsg, "listid", 18, 32); 
    m_params.readIntParam(pMsg, "type", 0, MAX_INTEGER);
}


/**
  * 检查参数，获取内部参数
  */
void FundQryBalanceOrder::excute() throw (CException)
{
    strncpy(m_orderInfo.Flistid,m_params["listid"],sizeof(m_orderInfo.Flistid)-1);
    m_orderInfo.Ftype = m_params.getInt("type");

    ST_ORDER_USER_RELA relData;
    memset(&relData,0,sizeof(relData));
    strncpy(relData.Flistid,m_params["listid"],sizeof(relData.Flistid)-1);
    relData.Ftype = m_params.getInt("type");
    
    if (false == queryOrderUserRelation(m_fund_conn, relData))
    {
        throw CException(ERR_FUND_QUERY_BA_ORDER, "queryOrderUserRelation error", __FILE__, __LINE__);
    }

    strncpy(m_orderInfo.Ftrade_id,relData.Ftrade_id,sizeof(m_orderInfo.Ftrade_id)-1);

    if (false == queryFundBalanceOrder(m_fund_conn, m_orderInfo,false))
    {
        throw CException(ERR_FUND_QUERY_BA_ORDER, "queryOrderUserRelation error", __FILE__, __LINE__);
    }
}


/**
  * 打包输出参数
  */
void FundQryBalanceOrder::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "uin", m_orderInfo.Fuin);
    CUrlAnalyze::setParam(rqst->odata, "uid",m_orderInfo.Fuid);
    CUrlAnalyze::setParam(rqst->odata, "trade_id", m_orderInfo.Ftrade_id);
    CUrlAnalyze::setParam(rqst->odata, "total_fee", m_orderInfo.Ftotal_fee);
    CUrlAnalyze::setParam(rqst->odata, "acc_time", m_orderInfo.Facc_time);
    CUrlAnalyze::setParam(rqst->odata, "spid", m_orderInfo.Fspid);
    CUrlAnalyze::setParam(rqst->odata, "state", m_orderInfo.Fstate);
    CUrlAnalyze::setParam(rqst->odata, "t1fetch_date", m_orderInfo.Ft1fetch_date);
    CUrlAnalyze::setParam(rqst->odata, "control_id", m_orderInfo.Fcontrol_id);
    CUrlAnalyze::setParam(rqst->odata, "total_acc_trans_id", m_orderInfo.Ftotal_acc_trans_id);
    CUrlAnalyze::setParam(rqst->odata, "flag", m_orderInfo.Fflag);
    CUrlAnalyze::setParam(rqst->odata, "inter_channel_id", m_orderInfo.Fstandby1);
    
    rqst->olen = strlen(rqst->odata);
    return;
}


