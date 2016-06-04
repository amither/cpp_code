
/**
  * FileName: fund_balance_recover_service.cpp
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-07-23
  * Description: 理财通余额回补
  */


#include "fund_commfunc.h"
#include "fund_balance_recover_service.h"

FundBalanceRecover::FundBalanceRecover(CMySQL* mysql)
{
    m_pFundCon = mysql; 
    m_sup_backer_fee = 0;
    memset(&m_recoverData,0,sizeof(m_recoverData));
    
}


/**
  * service step 1: 解析输入参数
  */
void FundBalanceRecover::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
   
    TRACE_DEBUG("[fund_balance_recover_service] receives: %s", szMsg);
    
    // 读取参数
    m_params.readStrParam(szMsg, "listid", 18, 32); 
    m_params.readIntParam(szMsg, "op_type", BA_RECOVER_REDEM, BA_RECOVER_BUY);
    m_params.readLongParam(szMsg, "total_fee", 0, MAX_LONG);
    m_params.readLongParam(szMsg, "total_buy_fee", 0, MAX_LONG);
    m_params.readLongParam(szMsg, "total_redem_fee", 0, MAX_LONG);
    m_params.readStrParam(szMsg, "purchaser_id", 1, 64); 
    m_params.readStrParam(szMsg, "bargainor_id", 1, 64); 
    m_params.readStrParam(szMsg, "token", 1, 64); 
    m_params.readStrParam(szMsg, "backer_transfer_listid", 18, 32); 
    m_params.readIntParam(szMsg, "req_type", FUND_BA_RECOVER_REQ, FUND_BA_RECOVER_CNF);
    m_params.readStrParam(szMsg, "trans_date", 8, 8);

    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);
}

/*
 * 检验token
 */
void FundBalanceRecover::CheckToken() throw (CException)
{
    stringstream ss;
    char buff[128] = {0};
    //uin|op_type|total_fee|cft_fetch_id|buy_id|total_acc_trans_id|key
    // 规则生成原串
    ss << m_params["listid"] << "|" ;
    ss << m_params["op_type"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << m_params["purchaser_id"] << "|" ;
    ss << m_params["bargainor_id"] << "|" ;
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
void FundBalanceRecover::CheckParams() throw (CException)
{
    if (m_params.getInt("req_type") == FUND_BA_RECOVER_REQ)
    {
        if (m_params.getInt("op_type") == BA_RECOVER_REDEM)
        {
            if (m_params.getLong("total_redem_fee") - m_params.getLong("total_buy_fee") != m_params.getLong("total_fee"))
            {
                throw EXCEPTION(ERR_BAD_PARAM, "input total_redem_fee  total_buy_fee total_fee check fail"); 
            }

            if (m_params.getString("bargainor_id") != m_fundBaCfg.Fbalance_spid_qqid)
            {
                throw EXCEPTION(ERR_BAD_PARAM, "input  bargainor_id check with Fbalance_spid_qqid  fail"); 
            }
            
        }
        else if (m_params.getInt("op_type") == BA_RECOVER_BUY)
        {
            if (m_params.getLong("total_buy_fee") - m_params.getLong("total_redem_fee") != m_params.getLong("total_fee"))
            {
                throw EXCEPTION(ERR_BAD_PARAM, "input total_buy_fee  total_redem_fee total_fee check fail"); 
            }
            if (m_params.getString("purchaser_id") != m_fundBaCfg.Fbalance_spid_qqid)
            {
                throw EXCEPTION(ERR_BAD_PARAM, "input  purchaser_id check with Fbalance_spid_qqid  fail"); 
            }
        }

        if (m_fundBaCfg.Ftotal_buy_balance_old != m_params.getLong("total_buy_fee"))
        {
            string errInfo= string("Ftotal_buy_balance_old=")+toString(m_fundBaCfg.Ftotal_buy_balance_old)+" not equal input total_buy_fee="+m_params.getString("total_buy_fee");
            TRACE_ERROR("%s",errInfo.c_str());
            alert(ERR_FUND_RECOVER_FEE_CHECK, errInfo);
        }

        if (m_fundBaCfg.Ftotal_redem_balance_old != m_params.getLong("total_redem_fee"))
        {
            string errInfo= string("Ftotal_redem_balance_old=")+toString(m_fundBaCfg.Ftotal_redem_balance_old)+" not equal input total_redem_fee="+m_params.getString("total_redem_fee");
            TRACE_ERROR("%s",errInfo.c_str());
            alert(ERR_FUND_RECOVER_FEE_CHECK, errInfo);
        }

        if (m_fundBaCfg.Ftotal_buy_balance_old < m_params.getLong("total_buy_fee"))
        {
            throw EXCEPTION(ERR_BAD_PARAM, "input  total_buy_fee check with Ftotal_buy_balance_old  fail"); 
        }
        
        if (m_fundBaCfg.Ftotal_redem_balance_old < m_params.getLong("total_redem_fee"))
        {
            throw EXCEPTION(ERR_BAD_PARAM, "input  total_redem_fee check with Ftotal_redem_balance_old  fail"); 
        }
        
    }
    else
    {
        if (m_fundBaCfg.Ftotal_baker_fee+m_fundBaCfg.Ftotal_baker_fee_old < m_params.getLong("total_fee")
            || m_params.getLong("total_fee") != m_recoverData.Ftotal_sup_backer_fee)
        {
            throw EXCEPTION(ERR_BAD_PARAM, "input Ffetch_backer_total_fee  Ftotal_baker_fee  Ftotal_baker_fee_old total_fee or  total_fee Ftotal_sup_backer_fee  check fail"); 
        }
    }
    
}


void FundBalanceRecover::UpdateFundBalanceConfig() throw (CException)
{
    TRACE_DEBUG("balance config :Ftotal_available_balance=%ld,Ftotal_redem_balance_old=%ld,"
                          "Ftotal_buy_balance_old=%ld,Ftotal_redem_balance=%ld,Ftotal_buy_balance=%ld,"
                          "Ftotal_baker_fee=%ld,Ftotal_baker_fee_old=%ld,Ftotal_t1fetch_fee=%ld,Ftotal_t1fetch_fee_old=%ld",
                      m_fundBaCfg.Ftotal_available_balance,m_fundBaCfg.Ftotal_redem_balance_old,m_fundBaCfg.Ftotal_buy_balance_old,
                      m_fundBaCfg.Ftotal_redem_balance,m_fundBaCfg.Ftotal_buy_balance,
                      m_fundBaCfg.Ftotal_baker_fee,m_fundBaCfg.Ftotal_baker_fee_old,m_fundBaCfg.Ftotal_t1fetch_fee,m_fundBaCfg.Ftotal_t1fetch_fee_old);

    strncpy(m_fundBaCfg.Fmodify_time,m_params["systime"],sizeof(m_fundBaCfg.Fmodify_time)-1);
    
    if (m_params.getInt("req_type") == FUND_BA_RECOVER_REQ)
    {
        m_sup_backer_fee = updateFundBalanceConfigForRecover(m_pFundCon, m_fundBaCfg, m_params.getLong("total_buy_fee"), m_params.getLong("total_redem_fee"), m_params.getLong("total_fee"));

        //记录余额总账户流水表

        recordTotalBalanceAccRoll(m_pFundCon,m_params["listid"],OP_TYPE_BA_RECOVER,m_params.getLong("total_fee")
                                                          ,m_params["systime"],m_fundBaCfg.Ftotal_available_balance
                                                          ,(m_params.getLong("total_redem_fee") >m_params.getLong("total_buy_fee")?1:0));
  
        if (m_sup_backer_fee > 0)
        {
            recordTotalBalanceAccRoll(m_pFundCon,m_params["backer_transfer_listid"],OP_TYPE_BA_BACKER_RECOVER,m_sup_backer_fee
                                                          ,m_params["systime"],m_fundBaCfg.Ftotal_available_balance+m_params.getLong("total_redem_fee")-m_params.getLong("total_buy_fee"),0);
        }
    }
    else
    {
        updateFundBalanceConfigForRecoverFetchBacker(m_pFundCon, m_fundBaCfg,m_params.getLong("total_fee"));
    }
}

void FundBalanceRecover::checkFundBalanceConfigTradeDate() throw (CException)
{
    string tradeDate = getTradeDate(m_pFundCon,m_params["systime"]);
    if (tradeDate.empty())
    {
        throw CException(ERR_FUND_QUERY_TRADE_DATE, "getTradeDate fail", __FILE__, __LINE__);
    }
    
    if (tradeDate > m_fundBaCfg.Ftrans_date)
    {
        updateFundbalanceConfigTradeDate(m_pFundCon,tradeDate,m_params["systime"]);
        memset(&m_fundBaCfg,0,sizeof(m_fundBaCfg));
        m_fundBaCfg.Fid = DEFAULT_BALNCE_CONFIG_FID;
        if (false == queryFundBalanceConfig(m_pFundCon,m_fundBaCfg,false))
        {
            throw CException(ERR_FUND_QUERY_BA_CONFIG, "query balance config fail", __FILE__, __LINE__);
        }
    }
}

void FundBalanceRecover::CheckRollList()throw (CException)
{
    strncpy(m_recoverData.Flistid,m_params["listid"],sizeof(m_recoverData.Flistid)-1);

    if (false == queryFundBalanceRecoverRoll(m_pFundCon, m_recoverData,  true))
    {
        if (m_params.getInt("req_type") == FUND_BA_RECOVER_CNF)
        {
            throw CException(ERR_BAD_PARAM, "queryFundBalanceRecoverRoll fail!", __FILE__, __LINE__);
        }
        return;
    }

    // 检查关键参数

    if (m_params.getInt("op_type") != m_recoverData.Ftype)
    {
        gPtrAppLog->error(" data exists, op_type is different! op_type in db[%d], op_type input[%d]", 
			m_recoverData.Ftype, m_params.getInt("op_type"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "data exists, op_type is different!", __FILE__, __LINE__);
    }
    
    if (m_params.getString("purchaser_id") != m_recoverData.Fpurchaser_id)
    {
        gPtrAppLog->error(" data exists, purchaser_id is different! purchaser_id in db[%s], purchaser_id input[%s]", 
			m_recoverData.Fpurchaser_id, m_params["purchaser_id"]);
        throw CException(ERR_REPEAT_ENTRY_DIFF, " data exists, purchaser_id is different!", __FILE__, __LINE__);
    }

    if (m_params.getString("bargainor_id") != m_recoverData.Fbargainor_id)
    {
        gPtrAppLog->error(" data exists, bargainor_id is different! bargainor_id in db[%s], bargainor_id input[%s]", 
			m_recoverData.Fpurchaser_id, m_params["bargainor_id"]);
        throw CException(ERR_REPEAT_ENTRY_DIFF, " data exists, bargainor_id is different!", __FILE__, __LINE__);
    }
    
    if (m_params.getInt("req_type") == FUND_BA_RECOVER_REQ)
    {
        if (m_params.getLong("total_fee") != m_recoverData.Ftotal_transfer_sup_fee)
        {
            gPtrAppLog->error(" data exists, total_fee is different! total_fee in db[%zd], total_fee input[%zd]", 
    			m_recoverData.Ftotal_transfer_sup_fee, m_params.getLong("total_fee"));
            throw CException(ERR_REPEAT_ENTRY_DIFF, " data exists, total_fee is different!", __FILE__, __LINE__);
        }
    }
    else
    {
        if (m_params.getLong("total_fee") != m_recoverData.Ftotal_sup_backer_fee)
        {
            gPtrAppLog->error(" data exists, total_fee is different! Ftotal_sup_backer_fee in db[%zd], total_fee input[%zd]", 
    			m_recoverData.Ftotal_sup_backer_fee, m_params.getLong("total_fee"));
            throw CException(ERR_REPEAT_ENTRY_DIFF, " data exists, total_fee is different!", __FILE__, __LINE__);
        }
    }


    if (m_params.getLong("total_buy_fee") != m_recoverData.Ftotal_buy_fee)
    {
        gPtrAppLog->error(" data exists, total_buy_fee is different! total_buy_fee in db[%zd], total_buy_fee input[%zd]", 
			m_recoverData.Ftotal_buy_fee, m_params.getLong("total_buy_fee"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, " data exists, total_buy_fee is different!", __FILE__, __LINE__);
    }

    if (m_params.getLong("total_redem_fee") != m_recoverData.Ftotal_redem_fee)
    {
        gPtrAppLog->error(" data exists, total_redem_fee is different! total_redem_fee in db[%zd], total_redem_fee input[%zd]", 
			m_recoverData.Ftotal_redem_fee, m_params.getLong("total_redem_fee"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, " data exists, total_redem_fee is different!", __FILE__, __LINE__);
    }

    if (m_recoverData.Fstate == BALANCE_RECOVER_OK)
    {
        throw CException(ERR_REPEAT_ENTRY, " data exists!", __FILE__, __LINE__);
    }

    if (m_params.getInt("req_type") == FUND_BA_RECOVER_CNF)
    {
        if (m_recoverData.Fstate != BALANCE_RECOVER_TRANSFER_SUPPLYED)
        {
            gPtrAppLog->error(" record state =%d invalid for fetch recover!",m_recoverData.Fstate);
            throw CException(ERR_INVALID_STATE, " record state invalid for fetch recover! ", __FILE__, __LINE__);
        }
    }
    else 
    {
        if (m_recoverData.Fstate == BALANCE_RECOVER_TRANSFER_SUPPLYED)
        {
            throw CException(ERR_REPEAT_ENTRY, " data exists!", __FILE__, __LINE__);
        }
    }
    
}

void FundBalanceRecover::RecordRollList()throw (CException)
{
    memset(&m_recoverData,0,sizeof(m_recoverData));
    strncpy(m_recoverData.Flistid,m_params["listid"],sizeof(m_recoverData.Flistid)-1);
    strncpy(m_recoverData.Fpurchaser_id,m_params["purchaser_id"],sizeof(m_recoverData.Fpurchaser_id)-1);
    strncpy(m_recoverData.Fbargainor_id,m_params["bargainor_id"],sizeof(m_recoverData.Fbargainor_id)-1);
    strncpy(m_recoverData.Fmodify_time,m_params["systime"],sizeof(m_recoverData.Fmodify_time)-1);
    strncpy(m_recoverData.Fcreate_time,m_params["systime"],sizeof(m_recoverData.Fcreate_time)-1);
    strncpy(m_recoverData.Facc_time,m_params["systime"],sizeof(m_recoverData.Facc_time)-1);
    m_recoverData.Ftotal_buy_fee = m_params.getLong("total_buy_fee");
    m_recoverData.Ftotal_redem_fee = m_params.getLong("total_redem_fee");
    m_recoverData.Ftotal_transfer_sup_fee = m_params.getLong("total_fee");
    m_recoverData.Ftotal_sup_backer_fee = m_sup_backer_fee;
    m_recoverData.Ftype= m_params.getInt("op_type");
    m_recoverData.Fstate= (m_sup_backer_fee>0?BALANCE_RECOVER_TRANSFER_SUPPLYED:BALANCE_RECOVER_OK);
    m_recoverData.Flstate= 1;
    m_recoverData.Fcur_type= 1;
    strncpy(m_recoverData.Fsup_backer_translist,m_params["backer_transfer_listid"],sizeof(m_recoverData.Fsup_backer_translist)-1);
    strncpy(m_recoverData.Fbacker_qqid,m_fundBaCfg.Ffetch_backer_qqid,sizeof(m_recoverData.Fbacker_qqid)-1);
    strncpy(m_recoverData.Ftrans_date,m_params["trans_date"],sizeof(m_recoverData.Ftrans_date)-1);
    
    insertFundFundBalanceRecoverRoll(m_pFundCon, m_recoverData);
}

void FundBalanceRecover::UpdateRollList()throw (CException)
{
    ST_BALANCE_RECOVER recoverData;
    memset(&recoverData,0,sizeof(recoverData));
    strncpy(recoverData.Flistid,m_params["listid"],sizeof(recoverData.Flistid)-1);
    strncpy(recoverData.Fmodify_time,m_params["systime"],sizeof(recoverData.Fmodify_time)-1);
    strncpy(recoverData.Fsup_backer_time,m_params["systime"],sizeof(recoverData.Fsup_backer_time)-1);

    recoverData.Fstate= BALANCE_RECOVER_OK;
    recoverData.Flstate= 1;
    
    updateFundFundBalanceRecoverRoll(m_pFundCon, recoverData);
}


/**
  * 检查参数，获取内部参数
  */
void FundBalanceRecover::excute() throw (CException)
{
    //校验token
    CheckToken();
    
    try
    {
        //启动事物
        m_pFundCon->Begin();

        m_fundBaCfg.Fid = DEFAULT_BALNCE_CONFIG_FID;
        //加锁查询余额配置表
        if (false == queryFundBalanceConfig(m_pFundCon,m_fundBaCfg,true))
        {
            throw CException(ERR_FUND_QUERY_BA_CONFIG, "query balance config fail", __FILE__, __LINE__);
        }

        //重入检查
        CheckRollList();

        //校验基金配置表中的交易日
        checkFundBalanceConfigTradeDate();

        //参数检查,注意这个函数必须在查询回补单之后
        CheckParams();

        //更新余额配置表
        UpdateFundBalanceConfig();

        if (m_params.getInt("req_type") == FUND_BA_RECOVER_CNF)
        {
            //更新回补流水单状态
            UpdateRollList();
        }
        else
        {
            //记录回补流水
            RecordRollList();
        }
        //提交事物
        m_pFundCon->Commit();
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

}


/**
  * 打包输出参数
  */
void FundBalanceRecover::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");

    CUrlAnalyze::setParam(rqst->odata, "total_sup_backer_fee", m_recoverData.Ftotal_sup_backer_fee);
    CUrlAnalyze::setParam(rqst->odata, "backer_transfer_listid", m_recoverData.Fsup_backer_translist);
    CUrlAnalyze::setParam(rqst->odata, "backer_qqid", m_recoverData.Fbacker_qqid);
    CUrlAnalyze::setParam(rqst->odata, "balance_sp_qqid", m_fundBaCfg.Fbalance_spid_qqid);
    CUrlAnalyze::setParam(rqst->odata, "backer_spid", m_fundBaCfg.Ffetch_backer_spid);
    
    rqst->olen = strlen(rqst->odata);
    return;
}









