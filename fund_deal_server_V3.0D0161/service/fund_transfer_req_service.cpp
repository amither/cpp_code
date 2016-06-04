/**
  * FileName: fund_transfer_req_service.cpp
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-05-16
  * Description: 份额转换请求接口
  */

#include "fund_commfunc.h"
#include "fund_transfer_req_service.h"

FundTransferReq::FundTransferReq(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
    memset(&m_fund_bind_orisp_acc, 0, sizeof(FundBindSp));
    memset(&m_fund_bind_newsp_acc, 0, sizeof(FundBindSp));
    memset(&m_transferOrder, 0, sizeof(m_transferOrder));
    memset(&m_fund_orisp_config, 0, sizeof(FundSpConfig));
    memset(&m_fund_newsp_config, 0, sizeof(FundSpConfig));
}

/**
  * service step 1: 解析输入参数
  */
void FundTransferReq::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    
    TRACE_DEBUG("[fund_transfer_req_service] receives: %s", szMsg);

    // 读取参数
    m_params.readStrParam(szMsg, "trade_id", 10,32);
    m_params.readStrParam(szMsg, "uin", 1, 64);
    m_params.readStrParam(szMsg, "fund_exchange_id", 10, 32); //转换单号
    m_params.readStrParam(szMsg, "ori_spid", 10, 15);
    m_params.readStrParam(szMsg, "new_spid", 10, 15);
    m_params.readStrParam(szMsg, "ori_fund_code", 1, 64);
    m_params.readStrParam(szMsg, "new_fund_code", 1, 64);
    m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "channel_id", 0, 64);
    m_params.readStrParam(szMsg, "buy_id", 10, 32);
    m_params.readStrParam(szMsg, "redem_id", 10, 32);
    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token

    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

}

/*
 * 生成基金注册用token
 */
string FundTransferReq::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照trade_id|uin|fund_exchange_id|Fori_spid|Fnew_spid|total_fee|key
    // 规则生成原串
    ss << m_params["trade_id"] << "|" ;
    ss << m_params["uin"] << "|" ;
    ss << m_params["fund_exchange_id"] << "|" ;
    ss << m_params["ori_spid"] << "|" ;
    ss << m_params["new_spid"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << gPtrConfig->m_AppCfg.transfer_reqkey;

    //TRACE_DEBUG("token src=%s", ss.str().c_str());
    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * 检验token
 */
void FundTransferReq::CheckToken() throw (CException)
{
	// 生成token
	string token = GenFundToken();

    if (StrUpper(m_params.getString("token")) != StrUpper(token))
    {   
	    TRACE_DEBUG("fund authen token check failed, input=%s", 
	                m_params.getString("token").c_str());
	    throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
    }   
}


/**
  * 检查参数，获取内部参数
  */
void FundTransferReq::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();
    if(m_params.getString("fund_exchange_id").substr(0,10) != m_params.getString("ori_spid"))
    {
        throw EXCEPTION(ERR_BAD_PARAM, "input fund_trans_id error check with ori_spid"); 
    }

    if(m_params.getString("buy_id").substr(0,10) != m_params.getString("new_spid"))
    {
        throw EXCEPTION(ERR_BAD_PARAM, "input buy_id error check with new_spid"); 
    }
    
    if(m_params.getString("redem_id").substr(0,10) != m_params.getString("ori_spid"))
    {
        throw EXCEPTION(ERR_BAD_PARAM, "input redem_id error check with ori_spid"); 
    }

    if(m_params.getLong("total_fee") > gPtrConfig->m_AppCfg.max_transfer_fee_one_time)
    {
        throw EXCEPTION(ERR_BAD_PARAM, "input total_fee exceed max fee limit"); 
    }
    
    //后三位同tradeid
    if(m_params.getString("fund_exchange_id").substr(m_params.getString("fund_exchange_id").length()-3,3) 
          != m_params.getString("trade_id").substr(m_params.getString("trade_id").length()-3,3))
    {
        throw EXCEPTION(ERR_BAD_PARAM, "input fund_trans_id error check with trade_id"); 
    }

    //检查spid 及fund_code 是否有效
    strncpy(m_fund_orisp_config.Fspid, m_params.getString("ori_spid").c_str(), sizeof(m_fund_orisp_config.Fspid) - 1);
    strncpy(m_fund_orisp_config.Ffund_code, m_params.getString("ori_fund_code").c_str(), sizeof(m_fund_orisp_config.Ffund_code) - 1);
    checkFundSpAndFundcode(m_pFundCon,m_fund_orisp_config, false);
	if(m_fund_orisp_config.Fclose_flag != CLOSE_FLAG_NORMAL)
	{	
		//定期产品暂时限制赎回
		throw EXCEPTION(ERR_BAD_PARAM, "Do not support the redemption of the Fund's");    
	}

    strncpy(m_fund_newsp_config.Fspid, m_params.getString("new_spid").c_str(), sizeof(m_fund_newsp_config.Fspid) - 1);
    strncpy(m_fund_newsp_config.Ffund_code, m_params.getString("new_fund_code").c_str(), sizeof(m_fund_newsp_config.Ffund_code) - 1);
    checkFundSpAndFundcode(m_pFundCon,m_fund_newsp_config, false);
}

/**
  * 执行申购请求
  */
void FundTransferReq::excute() throw (CException)
{
    try
    {
        CheckParams();

        // 检查基金账户记录 
        CheckFundBind();

        //检查基金账户绑定基金公司交易账户记录 
        CheckFundBindSpAcc();

        //检查用户余额
        checkUserBalance();

        //检查用户转换次数
        checkTransferTimes();

        //检查转出基金总份额限制
        checkSpRedemRate();

		// 检查基金是否可以进行申购 
		checkFundcodePurchaseValid(m_fund_newsp_config.Fbuy_valid);

		// 检查限额 
		checkFundcodeToScopeUpperLimit(m_fund_bind.Fuid,m_params["systime"],m_params.getLong("total_fee"), m_fund_newsp_config, true);

        // 开启事务 /
        m_pFundCon->Begin();

        // 查询并校验转换单 
        CheckFundTransfer();

		/* 检查用户是否可以购买定期理财产品 */
		checkPermissionBuyCloseFund(m_fund_bind.Ftrade_id, m_fund_newsp_config, m_params.getString("systime"), true);

        // 记录基金转换记录 
        RecordFundTransfer();

        // 提交事务 */
        m_pFundCon->Commit();
    }
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        m_pFundCon->Rollback();

        if (ERR_REPEAT_ENTRY != (unsigned)e.error())
        {
            throw;
        }
    }
}

/*
 * 查询基金账户是否存在
 */
void FundTransferReq::CheckFundBind() throw (CException)
{
    if(!QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }
    
    if (m_params.getString("trade_id") != m_fund_bind.Ftrade_id)
    {
        throw CException(ERR_FUNDBIND_NOTREG, "check fund bind record trade_id fail! ", __FILE__, __LINE__);
    }
}

/*
*检查是否绑定基金公司帐号，并且可交易
*/
void FundTransferReq::CheckFundBindSpAcc() throw (CException)
{
    strncpy(m_fund_bind_orisp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_orisp_acc.Ftrade_id) - 1);
    strncpy(m_fund_bind_orisp_acc.Fspid, m_params.getString("ori_spid").c_str(), sizeof(m_fund_bind_orisp_acc.Fspid) - 1);
    queryValidFundBindSp(m_pFundCon, m_fund_bind_orisp_acc, false);

    strncpy(m_fund_bind_newsp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_newsp_acc.Ftrade_id) - 1);
    strncpy(m_fund_bind_newsp_acc.Fspid, m_params.getString("new_spid").c_str(), sizeof(m_fund_bind_newsp_acc.Fspid) - 1);
    queryValidFundBindSp(m_pFundCon, m_fund_bind_newsp_acc, false);
}


/**
  * 检查基金转换记录是否已经生成
  */
void FundTransferReq::CheckFundTransfer() throw (CException)
{
    strncpy(m_transferOrder.Fchange_id,m_params["fund_exchange_id"],sizeof(m_transferOrder.Fchange_id)-1);
    bool bBuyTradeExist = queryFundTransfer(m_pFundCon, m_transferOrder, true);

    gPtrAppLog->debug("fund buy req trade record exist : %d", bBuyTradeExist);

    if(!bBuyTradeExist)
        return;

    // 检查关键参数
    if( (0 != strcmp(m_transferOrder.Fori_spid, m_params.getString("ori_spid").c_str())))
    {
        gPtrAppLog->error("fund trade exists, spid is different! ori_spid in db[%s], ori_spid input[%s]", 
			m_transferOrder.Fori_spid, m_params.getString("ori_spid").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, ori_spid is different!", __FILE__, __LINE__);
    }

    if( (0 != strcmp(m_transferOrder.Fnew_spid, m_params.getString("new_spid").c_str())))
    {
        gPtrAppLog->error("fund trade exists, spid is different! new_spid in db[%s], new_spid input[%s]", 
			m_transferOrder.Fnew_spid, m_params.getString("new_spid").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, new_spid is different!", __FILE__, __LINE__);
    }

    if(0 != strcmp(m_transferOrder.Ftrade_id, m_params.getString("trade_id").c_str()))
    {
        gPtrAppLog->error("fund trade exists, trade_id is different! trade_id in db[%s], trade_id input[%s] ", 
			m_transferOrder.Ftrade_id, m_params.getString("trade_id").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, trade_id is different!", __FILE__, __LINE__);
    }

    if(0 != strcmp(m_transferOrder.Fori_fund_code, m_params.getString("ori_fund_code").c_str()))
    {
        gPtrAppLog->error("fund trade exists, fund_code is different! ori_fund_code in db[%s], ori_fund_code input[%s] ", 
			m_transferOrder.Fori_fund_code, m_params.getString("ori_fund_code").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, ori_fund_code is different!", __FILE__, __LINE__);
    }

    if(0 != strcmp(m_transferOrder.Fnew_fund_code, m_params.getString("new_fund_code").c_str()))
    {
        gPtrAppLog->error("fund trade exists, fund_code is different! ori_fund_code in db[%s], new_fund_code input[%s] ", 
			m_transferOrder.Fnew_fund_code, m_params.getString("new_fund_code").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, new_fund_code is different!", __FILE__, __LINE__);
    }

    if(m_transferOrder.Ftotal_fee != m_params.getLong("total_fee"))
    {
        gPtrAppLog->error("fund trade exists, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_transferOrder.Ftotal_fee, m_params.getLong("total_fee"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, total_fee is different!", __FILE__, __LINE__);
    }

    if( m_params.getString("buy_id") != m_transferOrder.Fbuy_id)
    {
        gPtrAppLog->error("fund trade exists, Fbuy_id is different! buy_id in db[%s], Fbuy_id input[%s] ", 
			m_transferOrder.Fbuy_id, m_params.getString("Fbuy_id").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, Fbuy_id is different!", __FILE__, __LINE__);
    }

    if(m_params.getString("redem_id") != m_transferOrder.Fredem_id)
    {
        gPtrAppLog->error("fund trade exists, redem_id is different! redem_id in db[%s], redem_id input[%s] ", 
			m_transferOrder.Fredem_id, m_params.getString("redem_id").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, redem_id is different!", __FILE__, __LINE__);
    }

    // 记录存在，物理状态无效，报错
    if(LSTATE_INVALID == m_transferOrder.Flstate)
    {
        gPtrAppLog->error("fund trade exists, lstate is invalid. listid[%s], trade_id[%s] ", 
			m_transferOrder.Fchange_id, m_transferOrder.Ftrade_id);
        throw CException(ERR_TRADE_INVALID, "fund transfer exists, lstate is invalid. ", __FILE__, __LINE__);
    }

    // 只有初始态的单才能重入
    if(FUND_TRANSFER_INIT != m_transferOrder.Fstate)
    {
        throw CException(ERR_BUY_RECORD_INVALID, "fund transfer record state invalid. ", __FILE__, __LINE__);
    }

    throw CException(ERR_REPEAT_ENTRY,"fund tratransferde record already exist. ", __FILE__, __LINE__);

}

/**
  * 生成基金购买记录，状态: 等待付款
  */
void FundTransferReq::RecordFundTransfer()throw (CException)
{
    ST_TRANSFER_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRANSFER_FUND));

    strncpy(stRecord.Fchange_id, m_params.getString("fund_exchange_id").c_str(), sizeof(stRecord.Fchange_id)-1);
    strncpy(stRecord.Fori_spid, m_params.getString("ori_spid").c_str(), sizeof(stRecord.Fori_spid)-1);
    strncpy(stRecord.Fnew_spid, m_params.getString("new_spid").c_str(), sizeof(stRecord.Fnew_spid)-1);
    strncpy(stRecord.Fori_fund_code, m_params.getString("ori_fund_code").c_str(), sizeof(stRecord.Fori_fund_code)-1);
    strncpy(stRecord.Fnew_fund_code, m_params.getString("new_fund_code").c_str(), sizeof(stRecord.Fnew_fund_code)-1);
    strncpy(stRecord.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(stRecord.Ftrade_id)-1);
    
    stRecord.Ftotal_fee = m_params.getLong("total_fee");
    stRecord.Fstate = CREATE_INIT;
    stRecord.Flstate = LSTATE_VALID;
    stRecord.Fsubacc_state = CREATE_INIT;
    stRecord.Fcur_type = 1;

    strncpy(stRecord.Fbuy_id, m_params["buy_id"], sizeof(stRecord.Fbuy_id)-1);
    strncpy(stRecord.Fredem_id, m_params["redem_id"], sizeof(stRecord.Fredem_id)-1);

    strncpy(stRecord.Fcreate_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fcreate_time)-1);
    strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);
    strncpy(stRecord.Fchannel_id, m_params.getString("channel_id").c_str(), sizeof(stRecord.Fchannel_id)-1);
	
    insertFundTransfer(m_pFundCon, stRecord);
}


void FundTransferReq::checkUserBalance() throw (CException)
{
    //TODO 冻结金额是哪个字段,子账户只用来记账，暂时不存在冻结部分
    LONG balance = querySubaccBalance(m_fund_bind.Fuid,querySubaccCurtype(m_pFundCon, m_params.getString("ori_spid")));

    if(balance < m_params.getLong("total_fee"))
    {
        throw CException(ERR_CORE_USER_BALANCE, "not enough money", __FILE__, __LINE__);
    }
}


//检查用户转换次数
void FundTransferReq::checkTransferTimes() throw (CException)
{
    //如果有转换中的记录不允许新的转换请求
    if (checkIfExistTransferIngBill(m_pFundCon,m_params["trade_id"]))
    {
        throw CException(ERR_TRANSFERINT_EXIST, "user exist transfering record ,please try later!", __FILE__, __LINE__);
    }
    
    //白名单用户不限制次数
    if (gPtrConfig->m_AppCfg.transfer_limit_white_list.find(m_params["uin"]) != string::npos)
    {
        return;
    }
    
    ST_FUND_DYNAMIC data;
    memset(&data,0,sizeof(data));
    strncpy(data.Ftrade_id,m_params["trade_id"],sizeof(data.Ftrade_id)-1);
    if (queryFundDynamic(m_pFundCon,data,false) && m_params.getString("systime")<changeDatetimeFormat(string(data.Fredem_day)+"150000"))
    {
        if ((data.Fdyn_status_mask & USER_STOP_TRANSFER)==1  //TODO  后续如果要调整允许的最大次数，要15点之后才能生效
            || data.Fredem_times_day >= gPtrConfig->m_AppCfg.max_transfer_times_oneday)
        {
            throw CException(ERR_CANNOT_CHANGESP_AGAIN, "exceed max transfer times one day", __FILE__, __LINE__);
        }
    }
}

//检查转出基金总份额限制
void FundTransferReq::checkSpRedemRate() throw (CException)
{
    memset(&m_fund_newsp_config, 0, sizeof(FundSpConfig));
    strncpy(m_fund_newsp_config.Fspid, m_params["new_spid"], sizeof(m_fund_orisp_config.Fspid) - 1);
    strncpy(m_fund_newsp_config.Ffund_code, m_params["new_fund_code"], sizeof(m_fund_orisp_config.Ffund_code) - 1);
    if(!queryFundSpAndFundcodeConfig(m_pFundCon, m_fund_newsp_config, false))
    {
    	//不应该发生
    	throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
    }
    //无转入权限
    /*
    * 已经启用另外一个字段表示转入，建议这个地方去除，以免混淆
    if (m_fund_newsp_config.Fredem_valid&SP_NOT_ALLOW_TRANSFER_BUY)
    {
        throw EXCEPTION(ERR_TRANSFER_BUY_NOT_ALLOWED, "new spid transfer buy not allowed"); 
    }
    */

    memset(&m_fund_orisp_config, 0, sizeof(FundSpConfig));
    strncpy(m_fund_orisp_config.Fspid, m_params["ori_spid"], sizeof(m_fund_orisp_config.Fspid) - 1);
    strncpy(m_fund_orisp_config.Ffund_code, m_params["ori_fund_code"], sizeof(m_fund_orisp_config.Ffund_code) - 1);

    checkSpRedemRateLimit(m_pFundCon, m_fund_orisp_config,m_params["systime"],m_params.getLong("total_fee"));
   
}


/**
  * 打包输出参数
  */
void FundTransferReq::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());
    CUrlAnalyze::setParam(rqst->odata, "new_sp_user", m_fund_bind_newsp_acc.Fsp_user_id);
    CUrlAnalyze::setParam(rqst->odata, "new_sp_trans_id", m_fund_bind_newsp_acc.Fsp_trans_id);
    CUrlAnalyze::setParam(rqst->odata, "cre_id", m_fund_bind.Fcre_id);
    CUrlAnalyze::setParam(rqst->odata, "cre_type", m_fund_bind.Fcre_type);
    CUrlAnalyze::setParam(rqst->odata, "new_fund_code", m_params.getString("new_fund_code").c_str());
    CUrlAnalyze::setParam(rqst->odata, "buy_id", m_params.getString("buy_id").c_str());
    CUrlAnalyze::setParam(rqst->odata, "total_fee", m_params.getString("total_fee").c_str());
    rqst->olen = strlen(rqst->odata);
    return;
}


