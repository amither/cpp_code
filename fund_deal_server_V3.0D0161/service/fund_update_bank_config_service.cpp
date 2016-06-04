/**
  * FileName: fund_update_bank_config_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2014-04-16
  * Description: 基金交易服务 更新银行配置信息
  * 该接口为服务调用，不部署在relay上
  */

#include "fund_commfunc.h"
#include "fund_update_bank_config_service.h"

FundUpdateBankConfig::FundUpdateBankConfig(CMySQL* mysql)
{
    m_pFundCon = mysql;                 

}

/**
  * service step 1: 解析输入参数
  */
void FundUpdateBankConfig::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};
	
	char *pMsg = (char*)(rqst->idata);

    
    TRACE_DEBUG("[fund_update_bank_config_service] receives: %s", pMsg);

	m_params.readIntParam(pMsg, "bank_type", 1, MAX_INTEGER);
	m_params.readLongParam(pMsg, "once_quota", 1, MAX_LONG);
	m_params.readLongParam(pMsg, "day_quota", 1, MAX_LONG);
    m_params.readStrParam(pMsg, "client_ip", 1, 16);
    m_params.readStrParam(pMsg, "token", 1, 32);   // 接口token

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

}

/*
 * 生成基金注册用token
 */
string FundUpdateBankConfig::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照bank_type|once_quota|day_quota|key
    // 规则生成原串
    ss << m_params["bank_type"] << "|" ;
	ss << m_params["once_quota"] << "|" ;
	ss << m_params["day_quota"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * 检验token
 */
void FundUpdateBankConfig::CheckToken() throw (CException)
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
void FundUpdateBankConfig::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();
}

/**
  * 执行申购请求
  */
void FundUpdateBankConfig::excute() throw (CException)
{
	try
	{
	    CheckParams();

	     /* 开启事务 */
	    m_pFundCon->Begin();
		 
		 /* 更新银行配置信息 */
		UpdateBankConfig();

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


void FundUpdateBankConfig::UpdateBankConfig()
{
	FundBankConfig fundBankConfig; 
	memset(&fundBankConfig, 0, sizeof(FundBankConfig));

	fundBankConfig.Fonce_quota = m_params.getLong("once_quota");
	fundBankConfig.Fday_quota = m_params.getLong("day_quota");
	fundBankConfig.Fbank_type = m_params.getInt("bank_type");
	strncpy(fundBankConfig.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundBankConfig.Fmodify_time) - 1);

	updateFundBankConfig(m_pFundCon, fundBankConfig);

	//此处不需要更新ckv,由银行同步批跑批量更新
	
}


/**
  * 打包输出参数
  */
void FundUpdateBankConfig::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");

    rqst->olen = strlen(rqst->odata);
    return;
}


