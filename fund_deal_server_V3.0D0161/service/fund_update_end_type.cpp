/**
  * FileName: fund_update_end_type.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2014-05-27
  * Description: 基金交易服务 变更定期产品预约赎回信息 源文件
  */

#include "fund_commfunc.h"
#include "fund_update_end_type.h"

FundUpdateEndType::FundUpdateEndType(CMySQL* mysql)
{
    m_pFundCon = mysql;    
	memset(&m_fundCloseTrans, 0, sizeof(FundCloseTrans));

}

/**
  * service step 1: 解析输入参数
  */
void FundUpdateEndType::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

	// 要保留请求数据，抛差错使用
    m_request = rqst;

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_update_end_type] receives: %s", szMsg);
	
	m_params.readStrParam(szMsg, "trade_id", 1, 32);
	m_params.readStrParam(szMsg, "fund_code", 1, 64);
	m_params.readLongParam(szMsg, "close_listid", 1, MAX_LONG);

    m_params.readIntParam(szMsg, "user_end_type", 0, 3);
	m_params.readIntParam(szMsg, "end_sell_type", 0, 3);
	m_params.readLongParam(szMsg,"end_plan_amt",0, MAX_LONG);
	m_params.readStrParam(szMsg, "end_transfer_spid", 0, 15);
	m_params.readStrParam(szMsg, "end_transfer_fundcode", 0, 64);

    m_params.readStrParam(szMsg, "client_ip", 0, 16);
    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

}

/*
 * 生成基金注册用token
 */
string FundUpdateEndType::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照trade_id|fund_code|close_listid|user_end_type|end_sell_type|end_plan_amt|end_transfer_spid|end_transfer_fundcode|key
    // 规则生成原串
    ss << m_params["trade_id"] << "|" ;
	ss << m_params["fund_code"] << "|" ;
	ss << m_params["close_listid"] << "|" ;
    ss << m_params["user_end_type"] << "|" ;
    ss << m_params["end_sell_type"] << "|" ;
    ss << m_params["end_plan_amt"] << "|" ;
	ss << m_params["end_transfer_spid"] << "|" ;
	ss << m_params["end_transfer_fundcode"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * 检验token
 */
void FundUpdateEndType::CheckToken() throw (CException)
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
void FundUpdateEndType::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();

	// 设置金额默认值
	if(m_params.getString("end_plan_amt").empty()){
		m_params.setParam("end_plan_amt",MIN_INTEGER);
	}

	if(0 == m_params.getInt("user_end_type") && 0 == m_params.getInt("end_sell_type"))
	{
		throw EXCEPTION(ERR_BAD_PARAM, "user_end_type and end_sell_type not found."); 
	}

	if(CLOSE_FUND_SELL_TYPE_ANOTHER_FUND == m_params.getInt("end_sell_type"))
	{
		CHECK_PARAM_EMPTY("end_transfer_spid"); 
		CHECK_PARAM_EMPTY("end_transfer_fundcode"); 
	}

	if(CLOSE_FUND_END_TYPE_PATRIAL_REDEM == m_params.getInt("user_end_type") && 0 == m_params.getLong("end_plan_amt"))
	{
		throw EXCEPTION(ERR_BAD_PARAM, "user_end_type not found, or empty"); 
	}

}

/**
  * 执行变更
  */
void FundUpdateEndType::excute() throw (CException)
{
    try
    {
        CheckParams();

		/* 开启事务 */
		m_pFundCon->Begin();

		checkFundCloseTrans();

        updateFundCloseTransEndType();

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

void FundUpdateEndType::checkFundCloseTrans()  throw (CException)
{
	m_fundCloseTrans.Fid = m_params.getLong("close_listid");
	strncpy(m_fundCloseTrans.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fundCloseTrans.Ftrade_id) - 1);
	if(!queryFundCloseTrans(m_pFundCon, m_fundCloseTrans, true))
	{
		throw EXCEPTION(ERR_BAD_PARAM, "trans not exist."); 
	}

	if(m_params.getString("trade_id") != m_fundCloseTrans.Ftrade_id)
	{	
		throw CException(ERR_BASE_INFO_DIFF, "trade_id is different", __FILE__, __LINE__);	
	}
	
	if(m_params.getString("fund_code") != m_fundCloseTrans.Ffund_code)
	{	
		throw CException(ERR_BASE_INFO_DIFF, "fund_code is different", __FILE__, __LINE__);	
	}

	if(m_fundCloseTrans.Fcurrent_total_fee < m_params.getLong("end_plan_amt"))
	{
		throw EXCEPTION(ERR_BAD_PARAM, "end_plan_amt is greater than current_total_fee."); 
	}

	//超过预约截止日不允许变更
	if(m_fundCloseTrans.Fbook_stop_date < calculateFundDate(m_params.getString("systime")))
	{
		throw EXCEPTION(ERR_OVER_BOOK_STOP_DATE, "over appointment redemption deadline."); 
	}

	// 数据完全一致不更新
	if(m_fundCloseTrans.Fuser_end_type==m_params.getInt("user_end_type")&&
		m_fundCloseTrans.Fend_sell_type==m_params.getInt("end_sell_type")&&
		m_fundCloseTrans.Fend_plan_amt==m_params.getLong("end_plan_amt")&&
		0==strcmp(m_fundCloseTrans.Fend_transfer_fundcode,m_params.getString("end_transfer_fundcode").c_str())&&
		0==strcmp(m_fundCloseTrans.Fend_transfer_spid,m_params.getString("end_transfer_spid").c_str()))
	{		
		throw EXCEPTION(ERR_REPEAT_ENTRY, "repeat entry");		
	}

	
}


void FundUpdateEndType::updateFundCloseTransEndType() 
{
	FundCloseTrans fundCloseTrans;

	strncpy(fundCloseTrans.Ftrade_id, m_fundCloseTrans.Ftrade_id, sizeof(fundCloseTrans.Ftrade_id) - 1);
	fundCloseTrans.Fid = m_fundCloseTrans.Fid;
	fundCloseTrans.Fuser_end_type = m_params.getInt("user_end_type"); 
	fundCloseTrans.Fend_sell_type = m_params.getInt("end_sell_type"); 
	fundCloseTrans.Fend_plan_amt = m_params.getLong("end_plan_amt");
	strncpy(fundCloseTrans.Fend_transfer_fundcode, m_params.getString("end_transfer_fundcode").c_str(), sizeof(fundCloseTrans.Fend_transfer_fundcode) - 1);
	strncpy(fundCloseTrans.Fend_transfer_spid, m_params.getString("end_transfer_spid").c_str(), sizeof(fundCloseTrans.Fend_transfer_spid) - 1);
	strncpy(fundCloseTrans.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundCloseTrans.Fmodify_time) - 1);
	
	updateFundCloseTransById(m_pFundCon, fundCloseTrans);

	setFundCloseTransToKV(m_fundCloseTrans.Ftrade_id, m_fundCloseTrans.Ffund_code);
	
} 


/**
  * 打包输出参数
  */
void FundUpdateEndType::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
  
    rqst->olen = strlen(rqst->odata);
    return;
}


