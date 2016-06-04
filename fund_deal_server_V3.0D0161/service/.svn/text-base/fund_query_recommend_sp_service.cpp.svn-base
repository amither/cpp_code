/**
  * FileName: fund_query_recommend_sp_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-11-21
  * Description: 基金交易服务 查询是否可进行份额转换接口
  */

#include "fund_commfunc.h"
#include "fund_query_recommend_sp_service.h"

FundQueryRecommendSp::FundQueryRecommendSp(CMySQL* mysql)
{
    m_pFundCon = mysql;       

}

/**
  * service step 1: 解析输入参数
  */
void FundQueryRecommendSp::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
	char *pMsg = (char*)(rqst->idata);

    // 读取参数
    m_params.readStrParam(pMsg, "uin", 5, 64);
	m_params.readIntParam(pMsg, "channel_id", 1,3);
	

}


/**
  * 执行
  */
void FundQueryRecommendSp::excute() throw (CException)
{	

	string spid;
	if(OP_TYPE_WX == m_params.getInt("channel_id"))
	{
		spid= getWxRecommendSp();   
	}
	else
	{
		spid= getCftQQRecommendSp(); 
	}
	
	m_params.setParam("spid",spid);
	
}

string FundQueryRecommendSp::getWxRecommendSp()  throw (CException)
{
	// 选择推荐基金公司顺序: 1.白名单最优先； 2.按uin尾号选取；3.按基金公司收益率
	if(m_params.getString("uin").size() < 14)
	{
		throw EXCEPTION(ERR_BAD_PARAM, "input uin error"); 
	}

	string uin = m_params.getString("uin"); //****@aa.tenpay.com
	uin = uin.erase(uin.length() - 14, 14);

	//白名单
	map<string, string>::iterator iter = gPtrConfig->m_WxRecommendSpCfg.rec_sp_white_list.find(uin);
	if(iter != gPtrConfig->m_WxRecommendSpCfg.rec_sp_white_list.end())
	{
		TRACE_DEBUG("uin=%s,spid=%s", uin.c_str(), iter->second.c_str());
		return iter->second;
	}

	//按uin尾号选取
	string uinTail = uin.substr(uin.size() -1);
	iter =  gPtrConfig->m_WxRecommendSpCfg.rec_sp_uin_tail_config.find(uinTail);
	if(iter != gPtrConfig->m_WxRecommendSpCfg.rec_sp_uin_tail_config.end())
	{
		TRACE_DEBUG("uin=%s,uinTail=%s,spid=%s", uin.c_str(), uinTail.c_str(), iter->second.c_str());
		return iter->second;
	}

	//按基金公司收益率选取
	FundProfitRate fundProfitRate;
	memset(&fundProfitRate, 0, sizeof(FundProfitRate));
	 
	if(getHighestProfitRateSpFromKV(fundProfitRate))
	{
		return fundProfitRate.Fspid;
	}

	//不应该发生
	throw EXCEPTION(FUND_SYS_ERROR, "system error"); 
	
}

string FundQueryRecommendSp::getCftQQRecommendSp()  throw (CException)
{
	// 选择推荐基金公司顺序: 1.白名单最优先； 2.按uin尾号选取；3.按基金公司收益率

	//白名单
	map<string, string>::iterator iter = gPtrConfig->m_CftqqRecommendSpCfg.rec_sp_white_list.find(m_params.getString("uin"));
	//if(iter != gPtrConfig->m_CftqqRecommendSpCfg.rec_sp_white_list.end() && checkSpValid(iter->second)) //减少查询压力，信任配置
	if(iter != gPtrConfig->m_CftqqRecommendSpCfg.rec_sp_white_list.end())
	{
		TRACE_DEBUG("uin=%s,spid=%s", m_params.getString("uin").c_str(), iter->second.c_str());
		return iter->second;
	}

	//按uin尾号选取
	string uin = m_params.getString("uin");
	string uinTail = uin.substr(uin.size() -1);
	iter =  gPtrConfig->m_CftqqRecommendSpCfg.rec_sp_uin_tail_config.find(uinTail);
	//if(iter != gPtrConfig->m_CftqqRecommendSpCfg.rec_sp_uin_tail_config.end() && checkSpValid(iter->second) ) //减少查询压力，信任配置
	if(iter != gPtrConfig->m_CftqqRecommendSpCfg.rec_sp_uin_tail_config.end() )
	{
		TRACE_DEBUG("uin=%s,uinTail=%s,spid=%s", uin.c_str(), uinTail.c_str(), iter->second.c_str());
		return iter->second;
	}

	//按基金公司收益率选取
	FundProfitRate fundProfitRate;
	memset(&fundProfitRate, 0, sizeof(FundProfitRate));
	 
	if(getHighestProfitRateSpFromKV(fundProfitRate))
	{
		return fundProfitRate.Fspid;
	}

	//不应该发生
	throw EXCEPTION(FUND_SYS_ERROR, "system error"); 
	
}

bool FundQueryRecommendSp::checkSpValid(string spid)
{
	try
	{
		//检查spid 及fund_code 是否有效
		FundSpConfig fundSpConfig;
		memset(&fundSpConfig, 0, sizeof(FundSpConfig));
		strncpy(fundSpConfig.Fspid, spid.c_str(), sizeof(fundSpConfig.Fspid) - 1);
		checkFundSpAndFundcode(m_pFundCon,fundSpConfig, true);//转换必须是有效的基金公司
	}
	catch (CException& e)
    {
        gPtrSysLog->normal("[%s][%d]DECLARE_SO_INIT: %d, %s", e.file(), e.line(), e.error(), e.what());
        return false;
    }

	return true;
}


/**
  * 打包输出参数
  */
void FundQueryRecommendSp::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
	CUrlAnalyze::setParam(rqst->odata, "spid", m_params.getString("spid").c_str()); //选出的推荐基金公司spid

    rqst->olen = strlen(rqst->odata);
    return;
}


