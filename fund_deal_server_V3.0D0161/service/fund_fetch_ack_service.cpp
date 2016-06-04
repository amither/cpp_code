/**
  * FileName: fund_fetch_ack_service.cpp
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-07-23
  * Description: 理财通余额提现确认
  */

#include "fund_commfunc.h"
#include "fund_fetch_ack_service.h"

FundFetchAck::FundFetchAck(CMySQL* mysql)
{
    m_pFundCon = mysql;
    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
    memset(&m_fetch_data,0, sizeof(ST_BALANCE_ORDER));

}

void FundFetchAck::parseInputMsg(TRPC_SVCINFO* rqst) throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);

    // 要保留请求数据，抛差错使用
    m_request = rqst;
    
    
    TRACE_DEBUG("[fund_fetch_ack_service] receives: %s", szMsg);

    // 读取参数
    m_params.readStrParam(szMsg, "uin", 1, 64);
    m_params.readIntParam(szMsg, "op_type", 1,MAX_INTEGER);
    m_params.readLongParam(szMsg, "total_fee", 1,MAX_LONG);
    m_params.readIntParam(szMsg, "cur_type", 1,1);
    m_params.readStrParam(szMsg, "spid", 0, 15);
    m_params.readStrParam(szMsg, "cft_fetch_id", 0, 32);
    m_params.readStrParam(szMsg, "buy_id", 0, 32);
    m_params.readStrParam(szMsg, "total_acc_trans_id", 21, 28);
    m_params.readStrParam(szMsg, "desc", 0, 255);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "channel_id", 0, 64);
    m_params.readIntParam(szMsg, "result_sign", 0,SUBACC_FETCH_RESULT_FAIL); //
    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token

    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

}


/*
 * 检验token
 */
void FundFetchAck::CheckToken() throw (CException)
{
    stringstream ss;
    char buff[128] = {0};
    //uin|op_type|total_fee|cft_fetch_id|buy_id|total_acc_trans_id|key
    // 规则生成原串
    ss << m_params["uin"] << "|" ;
    ss << m_params["op_type"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << m_params["cft_fetch_id"] << "|" ;
    ss << m_params["buy_id"] << "|" ;
    ss << m_params["total_acc_trans_id"] << "|" ;
    ss << m_params["result_sign"] << "|" ;
    ss << gPtrConfig->m_AppCfg.fetch_service_key;

    TRACE_DEBUG("token src=%s", ss.str().c_str());
    getMd5(ss.str().c_str(), ss.str().size(), buff);

    if (StrUpper(m_params.getString("token")) != StrUpper(buff))
    {   
	    TRACE_DEBUG("fund authen token check failed, input=%s,real=%s", 
	                m_params.getString("token").c_str(),buff);
	    throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
    }   
}



/**
  * 检查参数，获取内部参数
  */
void FundFetchAck::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();

    if (m_params.getInt("op_type") != OP_TYPE_BA_BUY
        && m_params.getInt("result_sign") !=  SUBACC_FETCH_RESULT_OK) //目前提现到银行卡只能成功不支持失败
    {
        throw EXCEPTION(ERR_BAD_PARAM, "input result_sign check fail"); 
    }
    
    if (m_params.getInt("op_type") == OP_TYPE_BA_FETCH || m_params.getInt("op_type") == OP_TYPE_BA_FETCH_T1)
    {
        if (m_params.getString("cft_fetch_id").length()<18)
        {
            throw EXCEPTION(ERR_BAD_PARAM, "input cft_fetch_id check fail"); 
        }
        m_params.setParam("listid", m_params["cft_fetch_id"]);
    }
    else if (m_params.getInt("op_type") == OP_TYPE_BA_BUY)
    {
        if (checkTransIdAndSpid(m_params["spid"],m_params["buy_id"]) == false)
        {
            throw EXCEPTION(ERR_BAD_PARAM, "input spid check with  buy_id error "); 
        }
        m_params.setParam("listid", m_params["buy_id"]);
    }
    else
    {
        throw EXCEPTION(ERR_BAD_PARAM, "input op_type check fail"); 
    }
    
}

/*
 * 查询基金账户是否存在
 */
void FundFetchAck::CheckFundBind() throw (CException)
{
    if (!QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }
}

void FundFetchAck::CheckFetchOrder()throw (CException)
{
    strncpy(m_fetch_data.Flistid,m_params["listid"],sizeof(m_fetch_data.Flistid)-1);
    strncpy(m_fetch_data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_fetch_data.Ftrade_id)-1);
    m_fetch_data.Ftype = m_params.getInt("op_type");
    if (false == queryFundBalanceOrder(m_pFundCon, m_fetch_data,  false))
    {
        throw CException(ERR_BAD_PARAM, "fetch data not exists !", __FILE__, __LINE__);
    }

    // 检查关键参数

    if (m_params.getInt("op_type") != m_fetch_data.Ftype)
    {
        gPtrAppLog->error("fetch data exists, op_type is different! op_type in db[%d], op_type input[%d]", 
			m_fetch_data.Ftype, m_params.getInt("op_type"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fetch data exists, op_type is different!", __FILE__, __LINE__);
    }
    
    if (m_params.getString("uin") != m_fetch_data.Fuin)
    {
        gPtrAppLog->error("fetch data exists, uin is different! uin in db[%s], uin input[%s]", 
			m_fetch_data.Fuin, m_params["uin"]);
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fetch data exists, uin is different!", __FILE__, __LINE__);
    }

    if (m_params.getLong("total_fee") != m_fetch_data.Ftotal_fee)
    {
        gPtrAppLog->error("fetch data exists, total_fee is different! total_fee in db[%zd], total_fee input[%zd]", 
			m_fetch_data.Ftotal_fee, m_params.getLong("total_fee"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fetch data exists, total_fee is different!", __FILE__, __LINE__);
    }

    if (m_params.getString("listid") != m_fetch_data.Flistid)
    {
        gPtrAppLog->error("fetch data exists, listid is different! listid in db[%s], listid input[%s]", 
			m_fetch_data.Flistid, m_params["listid"]);
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fetch data exists, listid is different!", __FILE__, __LINE__);
    }

    if (m_params.getString("total_acc_trans_id") != m_fetch_data.Ftotal_acc_trans_id)
    {
        gPtrAppLog->error("fetch data exists, total_acc_trans_id is different! total_acc_trans_id in db[%s], total_acc_trans_id input[%s]", 
			m_fetch_data.Ftotal_acc_trans_id, m_params["cft_transfer_id"]);
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fetch data exists, total_acc_trans_id is different!", __FILE__, __LINE__);
    }

    if ((m_fetch_data.Fstate == FUND_FETCH_OK && m_params.getInt("result_sign") == SUBACC_FETCH_RESULT_OK) 
        ||(m_fetch_data.Fstate == FUND_FETCH_REFUND && m_params.getInt("result_sign") == SUBACC_FETCH_RESULT_FAIL) )
    {
        throw CException(ERR_REPEAT_ENTRY, "fetch data exists!", __FILE__, __LINE__);
    }

    if (m_fetch_data.Fstate != FUND_FETCH_SUBACC_OK)
    {
        gPtrAppLog->error("fetch record state =%d invalid",m_fetch_data.Fstate);
        throw CException(ERR_INVALID_STATE, "fetch record state invalid. ", __FILE__, __LINE__);
    }
    
}
      

void FundFetchAck::UpdateFetchOrder()throw (CException)
{
    ST_BALANCE_ORDER data;
    memset(&data,0,sizeof(ST_BALANCE_ORDER));
    strncpy(data.Flistid,m_params["listid"],sizeof(data.Flistid)-1);
    data.Ftype = m_params.getInt("op_type");
    strncpy(data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(data.Ftrade_id)-1);

    strncpy(data.Fmodify_time,m_params["systime"],sizeof(data.Fmodify_time)-1);

    data.Fstate = (m_params.getInt("result_sign")==SUBACC_FETCH_RESULT_OK?FUND_FETCH_OK:FUND_FETCH_REFUND);
    if(m_params.getInt("op_type") == OP_TYPE_BA_BUY)
    	{
    	data.Fstandby2=BA_FETCH_NOTIFY;
    	}
    
    updateFundBalanceOrder(m_pFundCon,data);
}


void FundFetchAck::doSubaccFetchResult()throw (CException)
{
    gPtrAppLog->debug("doSubaccFetchResult, listid[%s],result_sign=%d", m_fetch_data.Fsubacc_trans_id,m_params.getInt("result_sign"));

    try
    {
        SubaccFetchResult(gPtrSubaccRpc, gPtrConfig->m_SysCfg.balance_sp_id, m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_fetch_data.Fsubacc_trans_id, m_params.getLong("total_fee"), m_params["systime"],m_params.getInt("result_sign"),CUR_FUND_BALANCE);
    }
    catch(CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
        
        //子账户不应该失败，所有加钱失败的都发给差错
        //如果支付回调超过一定间隔时间以上，子账户还报异常，发告警
        if ((time_t)(toUnixTime(m_fetch_data.Facc_time))+ gPtrConfig->m_AppCfg.paycb_overtime_inteval < (time(NULL)))	
        {
            char szErrMsg[256] = {0};
            snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.余额提现确认超过10分钟子账户仍未成功");        
            alert(ERR_BUDAN_TOLONG, szErrMsg);
            if(ERR_TYPE_MSG)
            {
                throw ; //差错补单一定时间未成功的发送告警后不再补单，避免死循环或发生雪崩,通过对账渠道进行补单
            }
        }
		
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
        
        // 子账户提现确认失败发差错
        callErrorRpc(m_request, gPtrSysLog);
        throw;
    }
    return ;
}

void FundFetchAck::CheckFundTrade()  throw (CException)
{
    if (m_params.getInt("op_type") != OP_TYPE_BA_BUY)
    {
        return;
    }

    // 没有购买记录，继续下一步
    ST_TRADE_FUND stTradeBuy;
    memset(&stTradeBuy,0,sizeof(ST_TRADE_FUND));
    if (false == QueryTradeFund(m_pFundCon, m_params.getString("buy_id").c_str(), 
		PURTYPE_BUY, &stTradeBuy, false))
    {
        gPtrAppLog->debug("fund buy  trade record not exist : %s", m_params.getString("buy_id").c_str());
        throw CException(ERR_BAD_PARAM, "fund buy  trade record not exis !", __FILE__, __LINE__);
    }

    
    gPtrAppLog->debug("fund buy req trade record exist  state = %d", stTradeBuy.Fstate);

    if (stTradeBuy.Fstate == PAY_OK ||stTradeBuy.Fstate ==PAY_ACK_SUC  ||stTradeBuy.Fstate == PURCHASE_SUC)
    {
        m_params.setParam("result_sign",SUBACC_FETCH_RESULT_OK);
        gPtrAppLog->debug("result_sign set to 1 pay ok");
    }
    else if (stTradeBuy.Fstate == PURCHASE_APPLY_REFUND || stTradeBuy.Fstate == PURCHASE_REFUND_SUC)
    {
        m_params.setParam("result_sign",SUBACC_FETCH_RESULT_FAIL);
        m_refund_desc = stTradeBuy.Fmemo;
        gPtrAppLog->debug("result_sign set to 2 refund");
    }
    else
    {
        throw CException(ERR_BAD_PARAM, "fund buy  trade record state invalid !", __FILE__, __LINE__);
    }
}



void FundFetchAck::CheckOrderState() throw (CException)
{
    ST_BALANCE_ORDER fetchData;
    memset(&fetchData,0,sizeof(fetchData));
    
    strncpy(fetchData.Flistid,m_params["listid"],sizeof(fetchData.Flistid)-1);
    strncpy(fetchData.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(fetchData.Ftrade_id)-1);
    fetchData.Ftype = m_params.getInt("op_type");
    if (false == queryFundBalanceOrder(m_pFundCon, fetchData,  true))
    {
        throw CException(ERR_BAD_PARAM, "fetch data not exists !", __FILE__, __LINE__);
    }

    if ((fetchData.Fstate == FUND_FETCH_OK && m_params.getInt("result_sign") == SUBACC_FETCH_RESULT_OK) 
        ||(fetchData.Fstate == FUND_FETCH_REFUND && m_params.getInt("result_sign") == SUBACC_FETCH_RESULT_FAIL) )
    {
        throw CException(ERR_REPEAT_ENTRY, "fetch data exists!", __FILE__, __LINE__);
    }

    if (fetchData.Fstate != FUND_FETCH_SUBACC_OK)
    {
        gPtrAppLog->error("fetch record state =%d invalid",m_fetch_data.Fstate);
        throw CException(ERR_INVALID_STATE, "fetch record state invalid. ", __FILE__, __LINE__);
    }
    
}


void FundFetchAck::checkTotalFetchNum()  throw (CException)
{
    FundBalanceConfig fundBaCfg;
    memset(&fundBaCfg,0,sizeof(FundBalanceConfig));
    fundBaCfg.Fid = DEFAULT_BALNCE_CONFIG_FID;
    //加锁查询余额配置表
    if (false == queryFundBalanceConfig(m_pFundCon,fundBaCfg,true))
    {
        throw CException(ERR_FUND_QUERY_BA_CONFIG, "query balance config fail", __FILE__, __LINE__);
    }
    strncpy(fundBaCfg.Fmodify_time,m_params["systime"],sizeof(fundBaCfg.Fmodify_time)-1);

    //确认时要通过流水单中的acctime获取交易日期
    string tradeDate = getTradeDate(m_pFundCon,m_fetch_data.Facc_time);
    if (tradeDate.empty())
    {
        throw CException(ERR_FUND_QUERY_TRADE_DATE, "getTradeDate fail", __FILE__, __LINE__);
    }

    
    TRACE_DEBUG("balance config :Ftotal_available_balance=%ld,Ftotal_redem_balance_old=%ld,"
                          "Ftotal_buy_balance_old=%ld,Ftotal_redem_balance=%ld,Ftotal_buy_balance=%ld,"
                          "Ftotal_baker_fee=%ld,Ftotal_baker_fee_old=%ld,Ftotal_t1fetch_fee=%ld,Ftotal_t1fetch_fee_old=%ld,tradeDate=%s",
                      fundBaCfg.Ftotal_available_balance,fundBaCfg.Ftotal_redem_balance_old,fundBaCfg.Ftotal_buy_balance_old,
                      fundBaCfg.Ftotal_redem_balance,fundBaCfg.Ftotal_buy_balance,
                      fundBaCfg.Ftotal_baker_fee,fundBaCfg.Ftotal_baker_fee_old,fundBaCfg.Ftotal_t1fetch_fee,fundBaCfg.Ftotal_t1fetch_fee_old,tradeDate.c_str());

    if (m_params.getInt("result_sign") == SUBACC_FETCH_RESULT_FAIL)
    {
        if (m_fetch_data.Ftype == OP_TYPE_BA_BUY) //余额申购退款
        {
            //恢复申购额度
            updateFundBalanceConfigForBuyRefund(m_pFundCon,fundBaCfg,m_fetch_data.Ftotal_fee,tradeDate);
            m_refund_desc = "fund deal refund";
        }
        else
        {
            throw CException(ERR_FUND_BA_FETCH_SYS_ERORR, "check result_sign and Ftype fail!", __FILE__, __LINE__);
        }
    }
    else if (m_fetch_data.Ftype == OP_TYPE_BA_FETCH_T1)
    {
        updateFundBalanceConfigForT1FetchCnf(m_pFundCon,fundBaCfg,m_fetch_data.Ftotal_fee,tradeDate);

        //记录余额总账户流水表
        recordTotalBalanceAccRoll(m_pFundCon,m_params["listid"],m_params.getInt("op_type")
                                                          ,m_fetch_data.Ftotal_fee,m_params["systime"],fundBaCfg.Ftotal_available_balance,0);

    }
}


void FundFetchAck::excute()  throw (CException)
{
    //参数检查
    CheckParams();

    // 检查基金账户记录 
    CheckFundBind();

    try
    {
        //检查基金交易单
        CheckFundTrade();
            
        //重入检查
        CheckFetchOrder();

        //子账户提现结果确认
        doSubaccFetchResult();
        
        //启动事物
        m_pFundCon->Begin();

        //加锁查询提现单
        CheckOrderState();
        
        //更新提现单状态为成功
        UpdateFetchOrder();

        //如果是解冻需要恢复基金配置表中的余额
        //如果是t+1提现确认需要减t+1总额度
        if (m_params.getInt("result_sign") == SUBACC_FETCH_RESULT_FAIL
            || (m_fetch_data.Ftype == OP_TYPE_BA_FETCH_T1))
        {
            checkTotalFetchNum();
        }
        
        m_pFundCon->Commit();
        
    }
    catch(CException &e)
    {
        m_pFundCon->Rollback();
        
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        if (ERR_REPEAT_ENTRY != (unsigned)e.error())
        {
            throw;
        }
    }
    
}

void FundFetchAck::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    if (m_params.getInt("op_type") == OP_TYPE_BA_BUY)
    {
        CUrlAnalyze::setParam(rqst->odata, "refund_flag", (m_params.getInt("result_sign") == SUBACC_FETCH_RESULT_FAIL?1:0));
        CUrlAnalyze::setParam(rqst->odata, "refund_desc", m_refund_desc.c_str());
    }
    rqst->olen = strlen(rqst->odata);
    return;
}




