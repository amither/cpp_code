/**
  * FileName: fund_fetch_req_service.cpp
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-07-23
  * Description: 理财通余额提现请求
  */

#include "fund_commfunc.h"
#include "fund_fetch_req_service.h"

FundFetchReq::FundFetchReq(CMySQL* mysql)
{
    m_pFundCon = mysql;
    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
    memset(&m_fetch_data,0, sizeof(ST_BALANCE_ORDER));
    memset(&m_fundBaCfg,0,sizeof(m_fundBaCfg));

    m_subAccErrCode = 0;
    m_bNeedUnFreeze = false;
    m_unFreeze_Errcode = ERR_FUND_FETCH_BALANCE_LACK;
    m_bNeedBackerFee = false;
}

void FundFetchReq::parseInputMsg(TRPC_SVCINFO* rqst) throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    
    
    TRACE_DEBUG("[fund_fetch_req_service] receives: %s", szMsg);

    // 读取参数
    m_params.readStrParam(szMsg, "uin", 1, 64);
    m_params.readIntParam(szMsg, "op_type", 1,MAX_INTEGER);
    m_params.readLongParam(szMsg, "total_fee", 1,MAX_LONG);
    m_params.readIntParam(szMsg, "cur_type", 1,1);
    m_params.readStrParam(szMsg, "spid", 0, 15);
    m_params.readStrParam(szMsg, "cft_fetch_id", 0, 32);
    m_params.readStrParam(szMsg, "buy_id", 0, 32);
    m_params.readStrParam(szMsg, "total_acc_trans_id", 0, 28);
    m_params.readStrParam(szMsg, "desc", 0, 255);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "channel_id", 0, 64);
    m_params.readStrParam(szMsg, "control_list", 0, 64);
    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token
    m_params.readIntParam(szMsg, "customer_force_flag", 0,1); //客服强制提现标记

    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

}


/*
 * 检验token
 */
void FundFetchReq::CheckToken() throw (CException)
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
    ss << gPtrConfig->m_AppCfg.fetch_service_key;

    TRACE_DEBUG("token src=%s", ss.str().c_str());
    getMd5(ss.str().c_str(), ss.str().size(), buff);

    if (StrUpper(m_params.getString("token")) != StrUpper(buff))
    {   
	    TRACE_DEBUG("fund authen token check failed, input=%s", 
	                m_params.getString("token").c_str());
	    throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
    }   
}



/**
  * 检查参数，获取内部参数
  */
void FundFetchReq::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();
    
    if (m_params.getInt("op_type") == OP_TYPE_BA_FETCH || m_params.getInt("op_type") == OP_TYPE_BA_FETCH_T1)
    {
        if (m_params.getString("cft_fetch_id").length()<18)
        {
            throw EXCEPTION(ERR_BAD_PARAM, "input cft_fetch_id check fail"); 
        }
        m_params.setParam("listid", m_params["cft_fetch_id"]);
        if (m_params.getInt("op_type") == OP_TYPE_BA_FETCH && m_params.getString("total_acc_trans_id").length()<21)
        {
            throw EXCEPTION(ERR_BAD_PARAM, "input total_acc_trans_id check fail"); 
        }
        
    }
    else if (m_params.getInt("op_type") == OP_TYPE_BA_BUY)
    {
        if (checkTransIdAndSpid(m_params["spid"],m_params["buy_id"]) == false)
        {
            throw EXCEPTION(ERR_BAD_PARAM, "input spid check with  buy_id error "); 
        }
        if (m_params.getString("buy_id").length()!=28) // 10位spid+8位日期 + 10位流水号
        {
            throw EXCEPTION(ERR_BAD_PARAM, "input spid check buy_id length fail "); 
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
void FundFetchReq::CheckFundBind() throw (CException)
{
    if (!QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }
    else
    {
        if(m_fund_bind.Flstate == LSTATE_FREEZE)
        {
            throw CException(ERR_ALREADY_FREEZE, "the user be frozen! ", __FILE__, __LINE__);
        }
    }
}

void FundFetchReq::CheckFetchOrder(bool needLockQuery)throw (CException)
{
    if (needLockQuery)
    {
        memset(&m_fetch_data,0,sizeof(m_fetch_data));
        strncpy(m_fetch_data.Flistid,m_params["listid"],sizeof(m_fetch_data.Flistid)-1);
        strncpy(m_fetch_data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_fetch_data.Ftrade_id)-1);
        m_fetch_data.Ftype = m_params.getInt("op_type");
        if (false == queryFundBalanceOrder(m_pFundCon, m_fetch_data,  true))
        {
            throw CException(ERR_BAD_PARAM, "CheckFetchOrder queryFundBalanceOrder not exist! ", __FILE__, __LINE__);
        }
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

    if ((m_fetch_data.Ftype== OP_TYPE_BA_FETCH ||m_fetch_data.Ftype== OP_TYPE_BA_FETCH_T1) 
         && m_params.getString("control_list") != m_fetch_data.Fcontrol_id)
    {
        gPtrAppLog->error("fetch data exists, control_list is different! control_list in db[%s], control_list input[%s]", 
			m_fetch_data.Fcontrol_id, m_params["control_list"]);
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fetch data exists, Fcontrol_id is different!", __FILE__, __LINE__);
    }

    if (m_fetch_data.Fstate == FUND_FETCH_SUBACC_OK || m_fetch_data.Fstate == FUND_FETCH_OK)
    {
        if (m_fetch_data.Fflag == 1) //用了垫资账户
        {
            m_bNeedBackerFee= true;
        }
        throw CException(ERR_REPEAT_ENTRY, "fetch data exists!", __FILE__, __LINE__);
    }

    if (m_fetch_data.Fstate != FUND_FETCH_INT)
    {
        gPtrAppLog->error("fetch record state =%d invalid",m_fetch_data.Fstate);
        throw CException(ERR_INVALID_STATE, "fetch record state invalid. ", __FILE__, __LINE__);
    }
    strncpy(m_fetch_data.Facc_time,m_params["systime"],sizeof(m_fetch_data.Facc_time)-1);
}

void FundFetchReq::RecordFetchOrder()throw (CException)
{
    m_pFundCon->Begin();
    
    strncpy(m_fetch_data.Flistid,m_params["listid"],sizeof(m_fetch_data.Flistid)-1);
    m_fetch_data.Ftype = m_params.getInt("op_type");
    strncpy(m_fetch_data.Fuin,m_params["uin"],sizeof(m_fetch_data.Fuin)-1);
    strncpy(m_fetch_data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_fetch_data.Ftrade_id)-1);
    m_fetch_data.Ftotal_fee= m_params.getInt("total_fee");

    strncpy(m_fetch_data.Fchannel_id,m_params["channel_id"],sizeof(m_fetch_data.Fchannel_id)-1);
    m_fetch_data.Fcur_type = m_params.getInt("cur_type");
    strncpy(m_fetch_data.Fspid,m_params["spid"],sizeof(m_fetch_data.Fspid)-1);
    strncpy(m_fetch_data.Ftotal_acc_trans_id,m_params["total_acc_trans_id"],sizeof(m_fetch_data.Ftotal_acc_trans_id)-1);
    strncpy(m_fetch_data.Fmemo,m_params["desc"],sizeof(m_fetch_data.Fmemo)-1);
    strncpy(m_fetch_data.Fcreate_time,m_params["systime"],sizeof(m_fetch_data.Fcreate_time)-1);
    strncpy(m_fetch_data.Fmodify_time,m_params["systime"],sizeof(m_fetch_data.Fmodify_time)-1);
    strncpy(m_fetch_data.Facc_time,m_params["systime"],sizeof(m_fetch_data.Facc_time)-1);
    strncpy(m_fetch_data.Fcontrol_id,m_params["control_list"],sizeof(m_fetch_data.Fcontrol_id)-1);

    m_params.setParam("subacc_fetch_id", GenerateIdsBySpid(""));
    
    strncpy(m_fetch_data.Fsubacc_trans_id,m_params["subacc_fetch_id"],sizeof(m_fetch_data.Fsubacc_trans_id)-1);
    m_fetch_data.Fstate = FUND_FETCH_INT;
    m_fetch_data.Fuid = m_fund_bind.Fuid;
    
    if (m_params.getInt("op_type") == OP_TYPE_BA_BUY)
    	{
    	 m_fetch_data.Fstandby2=BA_FETCH_NOT_NOTIFY;
    	}
	m_fetch_data.Ffetch_result=FETCH_RESULT_INIT;
    insertFundBalanceOrder(m_pFundCon,m_fetch_data);

    RecordRelationOrder();

    m_pFundCon->Commit();
}
        
void FundFetchReq::RecordRelationOrder()throw (CException)
{
    ST_ORDER_USER_RELA data;
    memset(&data,0,sizeof(ST_ORDER_USER_RELA));
    strncpy(data.Flistid,m_params["listid"],sizeof(data.Flistid)-1);
    data.Ftype = m_params.getInt("op_type");
    strncpy(data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(data.Ftrade_id)-1);
    strncpy(data.Fcreate_time,m_params["systime"],sizeof(data.Fcreate_time)-1);
    strncpy(data.Fmodify_time,m_params["systime"],sizeof(data.Fmodify_time)-1);
    
    insertOrderUserRelation(m_pFundCon,data);

    if (m_params.getString("total_acc_trans_id") != m_params["listid"] && (!m_params.getString("total_acc_trans_id").empty()))
    {
        strncpy(data.Flistid,m_params["total_acc_trans_id"],sizeof(data.Flistid)-1);
        insertOrderUserRelation(m_pFundCon,data);
    }
    
    if (m_params.getString("subacc_fetch_id") != m_params["listid"]
        && m_params.getString("subacc_fetch_id") != m_params["total_acc_trans_id"])
    {
        strncpy(data.Flistid,m_params["subacc_fetch_id"],sizeof(data.Flistid)-1);
        insertOrderUserRelation(m_pFundCon,data);
    }
}

void FundFetchReq::UpdateFetchOrderBackerFlag()throw (CException)
{
    if (m_bNeedBackerFee == false)
    {
        return;
    }
    ST_BALANCE_ORDER data;
    memset(&data,0,sizeof(ST_BALANCE_ORDER));
    strncpy(data.Flistid,m_params["listid"],sizeof(data.Flistid)-1);
    data.Ftype = m_params.getInt("op_type");
    strncpy(data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(data.Ftrade_id)-1);
    data.Fflag =1;
    updateFundBalanceOrder(m_pFundCon,data);
}

void FundFetchReq::UpdateFetchOrderSuc()throw (CException)
{
    ST_BALANCE_ORDER data;
    memset(&data,0,sizeof(ST_BALANCE_ORDER));
    strncpy(data.Flistid,m_params["listid"],sizeof(data.Flistid)-1);
    data.Ftype = m_params.getInt("op_type");
    strncpy(data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(data.Ftrade_id)-1);

    strncpy(data.Facc_time,m_params["systime"],sizeof(data.Facc_time)-1);
    strncpy(data.Fmodify_time,m_params["systime"],sizeof(data.Fmodify_time)-1);

    data.Fstate = FUND_FETCH_SUBACC_OK;
    
    if (m_params.getInt("op_type") == OP_TYPE_BA_FETCH_T1)
    {
        string trade_date,fund_vdate;
        getTradeDate(m_pFundCon,data.Facc_time, trade_date,fund_vdate);
        strncpy(data.Ft1fetch_date,fund_vdate.c_str(),sizeof(data.Ft1fetch_date)-1);
    }
    
    updateFundBalanceOrder(m_pFundCon,data);
}

void FundFetchReq::UpdateFetchOrderRefund()throw (CException)
{
    ST_BALANCE_ORDER data;
    memset(&data,0,sizeof(ST_BALANCE_ORDER));
    strncpy(data.Flistid,m_params["listid"],sizeof(data.Flistid)-1);
    data.Ftype = m_params.getInt("op_type");
    strncpy(data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(data.Ftrade_id)-1);

    strncpy(data.Facc_time,m_params["systime"],sizeof(data.Facc_time)-1);
    strncpy(data.Fmodify_time,m_params["systime"],sizeof(data.Fmodify_time)-1);

    data.Fstate = FUND_FETCH_REFUND;

    strncpy(data.Fmemo,m_unFreezeDesc.c_str(),sizeof(data.Fmemo)-1);
    
    updateFundBalanceOrder(m_pFundCon,data);
}
        
void FundFetchReq::doSubaccFetch()throw (CException)
{
    gPtrAppLog->debug("doSubaccFetch, listid[%s]  ", m_fetch_data.Fsubacc_trans_id);

    try
    {
        SubaccFetchReq(gPtrSubaccRpc, gPtrConfig->m_SysCfg.balance_sp_id, m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_fetch_data.Fsubacc_trans_id, m_params.getLong("total_fee"), m_fetch_data.Facc_time,CUR_FUND_BALANCE);
    }
    catch(CException& e)
    {
        m_subAccErrCode = e.error();
        m_subAccErrInfo = e.what();
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
        
        throw;
    }
}

void FundFetchReq::doSubaccFetchCancle()throw (CException)
{
    gPtrAppLog->debug("doSubaccFetchCancle, listid[%s],result_sign=2", m_fetch_data.Fsubacc_trans_id);

   
    SubaccFetchResult(gPtrSubaccRpc, gPtrConfig->m_SysCfg.balance_sp_id, m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_fetch_data.Fsubacc_trans_id, m_params.getLong("total_fee"), m_fetch_data.Facc_time,2,CUR_FUND_BALANCE);

    
    return ;
}

void FundFetchReq::checkTotalFetchNum()  throw (CException)
{
    m_fundBaCfg.Fid = DEFAULT_BALNCE_CONFIG_FID;
    //加锁查询余额配置表
    if (false == queryFundBalanceConfig(m_pFundCon,m_fundBaCfg,true))
    {
        throw CException(ERR_FUND_QUERY_BA_CONFIG, "query balance config fail", __FILE__, __LINE__);
    }
    strncpy(m_fundBaCfg.Fmodify_time,m_params["systime"],sizeof(m_fundBaCfg.Fmodify_time)-1);

    string tradeDate = getTradeDate(m_pFundCon,m_params["systime"]);
    if (tradeDate.empty())
    {
        throw CException(ERR_FUND_QUERY_TRADE_DATE, "getTradeDate fail", __FILE__, __LINE__);
    }

    
    TRACE_DEBUG("balance config :Ftotal_available_balance=%ld,Ftotal_redem_balance_old=%ld,"
                          "Ftotal_buy_balance_old=%ld,Ftotal_redem_balance=%ld,Ftotal_buy_balance=%ld,"
                          "Ftotal_baker_fee=%ld,Ftotal_baker_fee_old=%ld,Ftotal_t1fetch_fee=%ld,Ftotal_t1fetch_fee_old=%ld,tradeDate=%s",
                      m_fundBaCfg.Ftotal_available_balance,m_fundBaCfg.Ftotal_redem_balance_old,m_fundBaCfg.Ftotal_buy_balance_old,
                      m_fundBaCfg.Ftotal_redem_balance,m_fundBaCfg.Ftotal_buy_balance,
                      m_fundBaCfg.Ftotal_baker_fee,m_fundBaCfg.Ftotal_baker_fee_old,m_fundBaCfg.Ftotal_t1fetch_fee,m_fundBaCfg.Ftotal_t1fetch_fee_old,tradeDate.c_str());

    if (m_fetch_data.Ftype == OP_TYPE_BA_FETCH) //余额t+0提现
    { 
        try
        {
            //更新提现额度
            updateFundBalanceConfigForFetch(m_pFundCon,m_fundBaCfg,m_fetch_data.Ftotal_fee,tradeDate,m_bNeedBackerFee);
            if (m_bNeedBackerFee == false)
            {
                //记录余额总账户流水表
                recordTotalBalanceAccRoll(m_pFundCon,m_params["listid"],m_params.getInt("op_type")
                                                          ,m_fetch_data.Ftotal_fee,m_params["systime"],m_fundBaCfg.Ftotal_available_balance,0);
            }
        }
        catch(CException &e)
        {
            if (e.error() == ERR_FUND_FETCH_BALANCE_LACK)
            {
                TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
                m_bNeedUnFreeze =true;
                m_unFreezeDesc = "可用提现额不足";
                m_unFreeze_Errcode = e.error();
                return;
            }
            throw;
        }
    }
    else if (m_fetch_data.Ftype == OP_TYPE_BA_FETCH_T1)
    {
        try
        {
            //更新提现额度
            updateFundBalanceConfigForT1FetchReq(m_pFundCon,m_fundBaCfg,m_fetch_data.Ftotal_fee,tradeDate);
        }
        catch(CException &e)
        {
            if (e.error() == ERR_FUND_T1FETCH_BALANCE_LACK)
            {
                TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
                m_bNeedUnFreeze =true;
                m_unFreezeDesc = "t+1提现可用额不足";
                m_unFreeze_Errcode = e.error();
                return;
            }
            throw;
        }
    }
    else if (m_fetch_data.Ftype == OP_TYPE_BA_BUY) //余额申购
    {
        try
        {
            //更新提现额度
            updateFundBalanceConfigForBuy(m_pFundCon,m_fundBaCfg,m_fetch_data.Ftotal_fee,tradeDate);
        }
        catch(CException &e)
        {
            if (e.error() == ERR_FUND_BA_BUY_BALANCE_LACK)
            {
                TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
                m_bNeedUnFreeze =true;
                m_unFreezeDesc = "可用申购额不足";
                m_unFreeze_Errcode = e.error();
                return;
            }
            throw;
        }
    }

}

void FundFetchReq::CheckFetchExauLimit()throw (CException)
{
     if (m_params.getInt("op_type") != OP_TYPE_BA_FETCH)
     {
        return;
     }
     if (m_params.getInt("customer_force_flag") == 1) //客服强制提现不验证限额
     {
         return;
     }
     checkBalanceFetchExauAuthLimit(gPtrExauRpc,m_fund_bind.Fuid,m_params.getLong("total_fee"),m_fund_bind.Fcre_id,FUND_BA_FETCH_EXAU_REQ_TYPE);
}

void FundFetchReq::PreCheckFetchBalance()throw (CException)
{
    //只有t+0提现才检查停止t0提现开关
    if (m_params.getInt("op_type") != OP_TYPE_BA_FETCH)
    {
        return;
    }
    FundBalanceConfig fundBaCfg;
    memset(&fundBaCfg,0,sizeof(fundBaCfg));
    fundBaCfg.Fid = DEFAULT_BALNCE_CONFIG_FID;
    if (false == queryFundBalanceConfig(m_pFundCon,fundBaCfg,false))
    {
        throw CException(ERR_FUND_QUERY_BA_CONFIG, "query balance config fail", __FILE__, __LINE__);
    }
    
    
    TRACE_DEBUG("balance config :Fflag=%d",fundBaCfg.Fflag);

    if ((fundBaCfg.Fflag&FETCH_LOAN_OVER) == 1) //已经停止t+0提现
    {
        throw CException(ERR_FUND_FETCH_OVER_FULL, "stop t0 fetch already!", __FILE__, __LINE__);
    }
}

bool FundFetchReq::queryFetchOrder() throw (CException)
{
    strncpy(m_fetch_data.Flistid,m_params["listid"],sizeof(m_fetch_data.Flistid)-1);
    strncpy(m_fetch_data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_fetch_data.Ftrade_id)-1);
    m_fetch_data.Ftype = m_params.getInt("op_type");
    
    return queryFundBalanceOrder(m_pFundCon, m_fetch_data,  false);
}
    
void FundFetchReq::UpdateFetchExauLimit()
{
    if (m_params.getInt("op_type") != OP_TYPE_BA_FETCH)
    {
        return;
    }
     
    try
    {
        updateBalanceFetchExauAuthLimit(gPtrExauRpc,m_fund_bind.Fuid, m_params.getLong("total_fee"),m_fund_bind.Fcre_id,FUND_BA_FETCH_EXAU_REQ_TYPE);
    }
    catch(CException& e)
    {
        TRACE_ERROR("UpdateFetchExauLimit error.[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
    }
}

void FundFetchReq::excute()  throw (CException)
{
    //参数检查
    CheckParams();

    // 检查基金账户记录 
    CheckFundBind();
    
    try
    {
        if (false == queryFetchOrder())
        {
            //非重入情况对于t+0提现检查是否今日额度已经用完
            //对于重入情况不检查，所以该函数不能放到最前面
            PreCheckFetchBalance();

            //检查提现限额，重入情况不检查限额
            CheckFetchExauLimit();
            
            //插入余额提现请求记录
            //该操作单独启动一个事物，放在子账户操作之前
            RecordFetchOrder();
        }
        else
        {
            //已经存在提现单 则检查重入参数
            CheckFetchOrder(false);
        }

        //子账户虚拟提现冻结
        //子账户操作可能耗时比较长，放到mysql事物之外
        doSubaccFetch();

        //启动事物
        m_pFundCon->Begin();

        //订单加锁并校验状态
        CheckFetchOrder(true);

        //更新提现单状态，放在checkTotalFetchNum之前是为了减少锁基金配置表的时间
        UpdateFetchOrderSuc();
        
        //检查并累加提现额度,如果t+0额度不足则设置m_bNeedUnFreeze为true
        //如果需要提现垫资，设置m_bNeedBackerFee 为true
        checkTotalFetchNum();
        
        //少量极端异常情况下会出现垫资不足无法t+0提现，因此需要解冻余额子账户，并返回调用方失败
        if (m_bNeedUnFreeze)
        {
            //子账户提现失败解冻
            doSubaccFetchCancle();
            //更新余额流水单状态为提现失败解冻
            UpdateFetchOrderRefund();
        }
        else
        {
            //更新流水单中的提现垫资标记
            UpdateFetchOrderBackerFlag();
        }

        //提交事物
        m_pFundCon->Commit();
        
        if (false == m_bNeedUnFreeze)
        {
            //对于t+0提现，子账户冻结成功后，就可以累加exau限额，如果返回在确认接口中累加，会有并发问题。
            UpdateFetchExauLimit();
        }
		updateCKV();
    }
    catch(CException &e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        //回滚事物
        m_pFundCon->Rollback();

        if (ERR_REPEAT_ENTRY != (unsigned)e.error())
        {
            throw;
        }
    }
    //额度不足导致冻结后又解冻，需要给itg返回错误
    if (m_bNeedUnFreeze)
    {
        throw CException(m_unFreeze_Errcode, m_unFreezeDesc, __FILE__, __LINE__);
    }
    
}

void FundFetchReq::updateCKV() throw (CException)
{
	setCashInTransitCKV(m_pFundCon,m_fund_bind.Ftrade_id);
}

void FundFetchReq::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");

    //t+0提现操作需要返回垫资方账户信息
    if (m_params.getInt("op_type") == OP_TYPE_BA_FETCH)
    {
        if (m_fundBaCfg.Ffetch_backer_spid[0] ==0)
        {
            m_fundBaCfg.Fid = DEFAULT_BALNCE_CONFIG_FID;
            if (false == queryFundBalanceConfig(m_pFundCon,m_fundBaCfg,false))
            {
                throw CException(ERR_FUND_QUERY_BA_CONFIG, "query balance config fail", __FILE__, __LINE__);
            }
        }
        if (m_bNeedBackerFee == true)
        {
            CUrlAnalyze::setParam(rqst->odata, "backer_spid", m_fundBaCfg.Ffetch_backer_spid);
            CUrlAnalyze::setParam(rqst->odata, "backer_qqid", m_fundBaCfg.Ffetch_backer_qqid);
            CUrlAnalyze::setParam(rqst->odata, "need_backer", "1");
        }
        else
        {
            CUrlAnalyze::setParam(rqst->odata, "backer_spid", m_fundBaCfg.Fbalance_spid);
            CUrlAnalyze::setParam(rqst->odata, "backer_qqid", m_fundBaCfg.Fbalance_spid_qqid);
            CUrlAnalyze::setParam(rqst->odata, "need_backer", "0");
        }
        CUrlAnalyze::setParam(rqst->odata, "fetch_spid", m_fundBaCfg.Ffund_fetch_spid);
        CUrlAnalyze::setParam(rqst->odata, "fetch_qqid", m_fundBaCfg.Ffund_fetch_spid_qqid);
    }
    rqst->olen = strlen(rqst->odata);
    return;
}



