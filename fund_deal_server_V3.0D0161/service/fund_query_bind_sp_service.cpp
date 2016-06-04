/**
  * FileName: fund_query_bind_sp_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-9-19
  * Description: 基金交易服务 查询余额增值账户绑定信息 源文件
  */

#include "fund_commfunc.h"
#include "fund_query_bind_sp_service.h"




FundQueryBindSp::FundQueryBindSp(CMySQL* mysql)
{
    m_fund_conn = mysql;

    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
	memset(&m_fund_bind_sp_acc, 0, sizeof(FundBindSp));

    m_bind_exist = false;
	m_bind_spacc_exist=false;

	relation_exist = 0;

}


/**
  * service step 1: 解析输入参数
  */
void FundQueryBindSp::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};


    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_query_bind_sp_service] receives: %s", szMsg);

    // 读取参数
    m_params.readIntParam(szMsg, "uid", 0,MAX_INTEGER);
    m_params.readStrParam(szMsg, "uin", 0, 64);
    m_params.readStrParam(szMsg, "spid", 0, 15);
    m_params.readStrParam(szMsg, "client_ip", 0, 16);
    m_params.readStrParam(szMsg, "trade_id", 0, 32);
    m_params.readIntParam(szMsg, "query_type", 0, 1);// 1:返回交易账户
}

/**
  * 检查参数
  */
void FundQueryBindSp::CheckParams() throw (CException)
{	
	//多基金版本如果没传商户号，默认用华夏的，其它基金公司在多基金版本发布但不开启不再支持
	if(gPtrConfig->m_AppCfg.multi_sp_config == 0)
	{
		//不开启多账户版本
		m_params.setParam("spid", gPtrConfig->m_AppCfg.default_sp);
	}
	else
	{
		CHECK_PARAM_EMPTY("spid");   
	}

    if (m_params.getString("uin").empty() && m_params.getString("trade_id").empty())
    {
        throw EXCEPTION(ERR_BAD_PARAM, "uin and trade_id is empty");
    }
}

/**
  * 执行基金账户开户
  */
void FundQueryBindSp::excute() throw (CException)
{
    try
    {
        CheckParams();

        /* 检查基金账户记录 */
        CheckFundBind();

		/* 检查基金账户绑定基金公司交易账户记录 */
		CheckFundBindSpAcc();

    }
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        m_fund_conn->Rollback();

        if ((ERR_REPEAT_ENTRY != (unsigned)e.error()) 
          && (ERR_REGOK_ALREADY != (unsigned)e.error()))
        {
            throw;
        }
    }
}

/*
 * 查询基金账户是否存在
 */
void FundQueryBindSp::CheckFundBind() throw (CException)
{
    if (!m_params.getString("uin").empty())
    {
        m_bind_exist = QueryFundBindByUin(m_fund_conn, m_params.getString("uin"), &m_fund_bind, false);
    }
    else
    {
        m_bind_exist = QueryFundBindByTradeid(m_fund_conn, m_params["trade_id"], &m_fund_bind, false,true);
    }

	if(!m_bind_exist)
		return;

    // 记录存在，读出记录中的trade_id
    m_params.setParam("trade_id", m_fund_bind.Ftrade_id);

	if(m_params.getInt("uid") !=0 && m_fund_bind.Fuid !=0 && m_params.getInt("uid") != m_fund_bind.Fuid)
	{
		TRACE_ERROR("uid in db=%d diff with input=%d", 
					m_fund_bind.Fuid, m_params.getInt("uid"));
		throw EXCEPTION(ERR_USER_INFO_ERR, "uid in db diff with input.");
	}
}

void FundQueryBindSp::CheckFundBindSpAcc() throw (CException)
{
	if(!m_bind_exist)
	{
		return;
	}
	
	strncpy(m_fund_bind_sp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_sp_acc.Ftrade_id) - 1);
	
	strncpy(m_fund_bind_sp_acc.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_bind_sp_acc.Fspid) - 1);
	m_bind_spacc_exist = queryFundBindSp(m_fund_conn, m_fund_bind_sp_acc, false);
	
    if(!m_bind_spacc_exist)
    {
        return;
    }

	/*
	if(LSTATE_FREEZE == m_fund_bind_sp_acc.Flstate)
	{
		//账户被冻结不允许继续开户
		TRACE_ERROR("the fund bind sp account record has been frozen.");
        throw EXCEPTION(ERR_SP_ACC_FREEZE, "the fund bind sp account record has been frozen.");
	}
	*/

	if(m_fund_bind_sp_acc.Fstate == BIND_SPACC_SUC)
	{
		relation_exist = 1;
	}
		
}


/**
  * 打包输出参数
  */
void FundQueryBindSp::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "relation_exist", relation_exist);
	CUrlAnalyze::setParam(rqst->odata, "spid",m_fund_bind_sp_acc.Fspid);
	CUrlAnalyze::setParam(rqst->odata, "acc_type",m_fund_bind_sp_acc.Facct_type);
	CUrlAnalyze::setParam(rqst->odata, "state",m_fund_bind_sp_acc.Fstate);
	CUrlAnalyze::setParam(rqst->odata, "lstate",m_fund_bind_sp_acc.Flstate);
	CUrlAnalyze::setParam(rqst->odata, "uid",m_fund_bind.Fuid);
	CUrlAnalyze::setParam(rqst->odata, "trade_id",m_fund_bind.Ftrade_id);
    if (m_params.getInt("query_type") == 1)
    {
        CUrlAnalyze::setParam(rqst->odata, "sp_trans_id",m_fund_bind_sp_acc.Fsp_trans_id);
        CUrlAnalyze::setParam(rqst->odata, "sp_user_id",m_fund_bind_sp_acc.Fsp_user_id);
        CUrlAnalyze::setParam(rqst->odata, "uin",m_fund_bind.Fqqid);
    }
    rqst->olen = strlen(rqst->odata);
    return;
}


