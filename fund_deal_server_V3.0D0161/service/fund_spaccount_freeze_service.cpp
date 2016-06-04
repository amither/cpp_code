/**
  * FileName: fund_spaccount_freeze_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-8-28
  * Description: 基金交易服务 基金公司绑定账户冻结解冻 源文件
  */

#include "fund_commfunc.h"
#include "fund_spaccount_freeze_service.h"

FundSpAccFreeze::FundSpAccFreeze(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_fund_bind_sp_acc, 0, sizeof(FundBindSp));

    m_bind_spacc_exist =false;				
    m_optype = 0;                      

}

/**
  * service step 1: 解析输入参数
  */
void FundSpAccFreeze::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_redem_sp_req_service] receives: %s", szMsg);

	//商户号（必填）
    m_params.readStrParam(szMsg, "spid", 1, 15);
    //基金交易账号对应id（必填）
    m_params.readStrParam(szMsg, "trade_id", 1, 32);
    //用户在基金公司的交易账号（必填）
    m_params.readStrParam(szMsg, "sp_user_id", 1, 64);
    m_params.readStrParam(szMsg, "sp_trans_id", 1, 64);
    //操作类型：（必填）
    m_params.readIntParam(szMsg, "op_type", 1, 2);
    //说明原因（必填）
    m_params.readStrParam(szMsg, "desc", 1, 128);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

	m_optype = m_params.getInt("op_type");

}

/*
 * 生成基金注册用token
 */
string FundSpAccFreeze::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照trade_id|spid|sp_trans_id|op_type|key
    // 规则生成原串
    ss << m_params["trade_id"] << "|" ;
    ss << m_params["spid"] << "|" ;
    ss << m_params["sp_trans_id"] << "|" ;
    ss << m_params["op_type"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * 检验token
 */
void FundSpAccFreeze::CheckToken() throw (CException)
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
void FundSpAccFreeze::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();
}

/**
  * 执行申购请求
  */
void FundSpAccFreeze::excute() throw (CException)
{
    try
    {
        CheckParams();

         /* 开启事务 */
        m_pFundCon->Begin();
		 
		 /* 检查基金账户绑定基金公司交易账户记录 */
		CheckFundBindSpAcc();

		UpdateFundBindSpAccFreeze();

        /* 提交事务 */
        m_pFundCon->Commit();
    }
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        m_pFundCon->Rollback();

        if ((ERR_REPEAT_ENTRY != (unsigned)e.error()) 
          && (ERR_REGOK_ALREADY != (unsigned)e.error()))
        {
            throw;
        }
    }
}


/*
*检查是否绑定基金公司帐号，并且可交易
*/
void FundSpAccFreeze::CheckFundBindSpAcc() throw (CException)
{
	strncpy(m_fund_bind_sp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_sp_acc.Ftrade_id) - 1);
	strncpy(m_fund_bind_sp_acc.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_bind_sp_acc.Fspid) - 1);
	m_bind_spacc_exist = queryFundBindSp(m_pFundCon, m_fund_bind_sp_acc, true);
	
    if(!m_bind_spacc_exist)
    {
		TRACE_ERROR("the fund bind sp account record not exist.spid:%s",m_params.getString("spid").c_str());
		throw EXCEPTION(ERR_NOT_BIND_SP_ACC, "the fund bind sp account record not exist");
    }
	
	// 记录存在，读出记录中的主键imt_id
	m_params.setParam("imt_id", m_fund_bind_sp_acc.Fimt_id);

	// 检查关键参数
	if (!m_params.getString("sp_user_id").empty() && !string(m_fund_bind_sp_acc.Fsp_user_id).empty()
		&& m_params.getString("sp_user_id") != m_fund_bind_sp_acc.Fsp_user_id)
    {
        TRACE_ERROR("sp_user_id in db=%s diff with input=%s", 
                    m_fund_bind_sp_acc.Fsp_user_id, m_params.getString("sp_user_id").c_str());
        throw EXCEPTION(ERR_BIND_SPACC_INFO_DIFF, "sp_user_id in db diff with input");
    }
	if (!m_params.getString("sp_trans_id").empty() && !string(m_fund_bind_sp_acc.Fsp_trans_id).empty()
		&& m_params.getString("sp_trans_id") != m_fund_bind_sp_acc.Fsp_trans_id)
    {
        TRACE_ERROR("sp_trans_id in db=%s diff with input=%s", 
                    m_fund_bind_sp_acc.Fsp_trans_id, m_params.getString("sp_trans_id").c_str());
        throw EXCEPTION(ERR_BIND_SPACC_INFO_DIFF, "sp_trans_id in db diff with input");
    }

	if(LSTATE_FREEZE == m_fund_bind_sp_acc.Flstate && INF_FREEZE == m_optype)
	{
		//账户已被冻结
		TRACE_ERROR("the fund bind sp account has been frozen.");
        throw EXCEPTION(ERR_REPEAT_ENTRY, "the fund bind sp account record has been frozen.");
	}

	if(LSTATE_VALID == m_fund_bind_sp_acc.Flstate && INF_UNFREEZE == m_optype)
	{
		//账户未被冻结
		TRACE_ERROR("the fund bind sp account not been frozen.");
        throw EXCEPTION(ERR_REPEAT_ENTRY, "the fund bind sp account not been frozen.");
	}

}

void FundSpAccFreeze::UpdateFundBindSpAccFreeze()
{
	FundBindSp fund_bind_sp_acc;
	strncpy(fund_bind_sp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(fund_bind_sp_acc.Ftrade_id) - 1);       
	strncpy(fund_bind_sp_acc.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fund_bind_sp_acc.Fmodify_time)-1);
	strncpy(fund_bind_sp_acc.Fmemo, m_params.getString("desc").c_str(), sizeof(fund_bind_sp_acc.Fmemo) - 1);       
	fund_bind_sp_acc.Fimt_id = m_params.getLong("imt_id");
	fund_bind_sp_acc.Flstate = (INF_FREEZE == m_optype) ? LSTATE_FREEZE : LSTATE_VALID;

	updateFundBindSpFreeze(m_pFundCon, fund_bind_sp_acc);

	//更新缓存
	updateBindSpKvCache();

}

void FundSpAccFreeze::updateBindSpKvCache()
{
	setFundBindAllSpToKVFromDB(m_pFundCon,m_params.getString("trade_id"));

}


/**
  * 打包输出参数
  */
void FundSpAccFreeze::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());
	CUrlAnalyze::setParam(rqst->odata, "acc_time", m_params.getString("systime").c_str());

    rqst->olen = strlen(rqst->odata);
    return;
}


