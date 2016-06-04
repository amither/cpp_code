/**
  * FileName: fund_prepay_req_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-11-07
  * Description: 基金交易服务 非实名认证用户(暂无法开通理财账户)预支付请求接口
  */

#include "fund_commfunc.h"
#include "fund_prepay_req_service.h"

FundPrepayReq::FundPrepayReq(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_fund_prepay, 0, sizeof(FundPrepay));          

	m_fund_prepay_exist = false;

}

/**
  * service step 1: 解析输入参数
  */
void FundPrepayReq::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};
	
    char *pMsg = (char*)(rqst->idata);

    m_params.readStrParam(pMsg, "uin", 1, 64);
	m_params.readStrParam(pMsg, "openidA", 1, 64);
	m_params.readIntParam(pMsg, "acct_type", 0, 2);
	m_params.readStrParam(pMsg, "fund_trans_id", 1, 32);
	m_params.readStrParam(pMsg, "spid", 10, 15);
	m_params.readStrParam(pMsg, "fund_name", 0, 64);
	m_params.readStrParam(pMsg, "fund_code", 1, 64);
	m_params.readLongParam(pMsg, "total_fee", 1, MAX_LONG);
      m_params.readStrParam(pMsg, "channel_id", 0, 64);
      m_params.readIntParam(pMsg, "purpose", 0, MAX_INTEGER);
	//m_params.readStrParam(pMsg, "client_ip", 0, 16);
    //m_params.readStrParam(pMsg, "token", 1, 32);   // 接口token

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);
    // 增值券id,非必填
    m_params.readStrParam(pMsg, "coupon_id", 0,32);

}

/**
  * 检查参数，获取内部参数
  */
void FundPrepayReq::CheckParams() throw (CException)
{
	// 检查交易单和spid相匹配
    if(m_params.getString("fund_trans_id").substr(0,10) != m_params.getString("spid"))
	{
		throw EXCEPTION(ERR_BAD_PARAM, "input fund_trans_id error"); 
	}	

	if(m_params.getInt("acct_type") == 0)
	{
		m_params.setParam("acct_type", 1);
	}
}

/**
  * 执行申购请求
  */
void FundPrepayReq::excute() throw (CException)
{
    try
    {
        CheckParams();

         /* 开启事务 */
        m_pFundCon->Begin();
		 
		CheckFundPrepay();

		RecordFundPrepay();

		//记录申购单
		RecordFundTrade();

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
*检查预支付订单信息
*/
void FundPrepayReq::CheckFundPrepay() throw (CException)
{
	strncpy(m_fund_prepay.Flistid, m_params.getString("fund_trans_id").c_str(), sizeof(m_fund_prepay.Flistid) - 1);
	m_fund_prepay_exist = queryFundPrepay(m_pFundCon, m_fund_prepay, true);
	
    if(!m_fund_prepay_exist)
    {
		return;
    }

	// 检查关键参数
    if( (0 != strcmp(m_fund_prepay.Fspid, m_params.getString("spid").c_str())))
    {
        TRACE_ERROR("prepay record exists, spid is different! spid in db[%s], spid input[%s]", 
			m_fund_prepay.Fspid, m_params.getString("spid").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "prepay record exists, spid is different!", __FILE__, __LINE__);
    }

    if(0 != strcmp(m_fund_prepay.Fuin, m_params.getString("uin").c_str()))
    {
        TRACE_ERROR("prepay record exists, uin is different! uin in db[%s], uin input[%s] ", 
			m_fund_prepay.Fuin, m_params.getString("uin").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "prepay record exists, uin is different!", __FILE__, __LINE__);
    }

    if(0 != strcmp(m_fund_prepay.Ffund_code, m_params.getString("fund_code").c_str()))
    {
        TRACE_ERROR("prepay record exists, fund_code is different! fund_code in db[%s], fund_code input[%s] ", 
			m_fund_prepay.Ffund_code, m_params.getString("fund_code").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "prepay record exists, fund_code is different!", __FILE__, __LINE__);
    }

    if(m_fund_prepay.Ftotal_fee != m_params.getLong("total_fee"))
    {
        TRACE_ERROR("prepay record exists, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_fund_prepay.Ftotal_fee, m_params.getLong("total_fee"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "prepay record exists, total_fee is different!", __FILE__, __LINE__);
    }

	if(PREPAY_INIT != m_fund_prepay.Fstate)
	{
		//非初始状态不能重入
		TRACE_ERROR("the prepay record not init state.");
        throw CException(ERR_NOR_PREPAY_INIT, "the prepay record not init state,cannot repeat enter.", __FILE__, __LINE__);
	}

	throw CException(ERR_REPEAT_ENTRY, "the prepay record has exist! ", __FILE__, __LINE__);

}

void FundPrepayReq::RecordFundPrepay()
{
	if(m_fund_prepay_exist)
	{
		return;
	}
	
	FundSpConfig data;
	strncpy(data.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(data.Ffund_code) - 1);
	strncpy(data.Fspid, m_params.getString("spid").c_str(), sizeof(data.Fspid) - 1);
	if(!queryFundSpAndFundcodeConfig(m_pFundCon,data,false))
	{
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error");
	}
	if(m_params.getLong("total_fee") < data.Fbuy_lower_limit || 0 != m_params.getLong("total_fee") % data.Fbuy_add_limit)
	{
		//小于购买金额限制或者不满足步长要求，报错
		throw EXCEPTION(ERR_BAD_PARAM, "input total_fee error");
	}

	FundPrepay stRecord;
	memset(&stRecord, 0, sizeof(FundPrepay));
	
	strncpy(stRecord.Flistid, m_params.getString("fund_trans_id").c_str(), sizeof(stRecord.Flistid) - 1);  
	strncpy(stRecord.Fuin, m_params.getString("uin").c_str(), sizeof(stRecord.Fuin) - 1);  
	strncpy(stRecord.Fopenid, m_params.getString("openidA").c_str(), sizeof(stRecord.Fopenid) - 1);  
	stRecord.Facct_type= m_params.getInt("acct_type");
	//stRecord.Fcre_type = m_params.getInt("cre_type");
	//strncpy(stRecord.Fcre_id, m_params.getString("cre_id").c_str(), sizeof(stRecord.Fcre_id) - 1);  
	//strncpy(stRecord.Ftrue_name, m_params.getString("true_name").c_str(), sizeof(stRecord.Ftrue_name) - 1);  
	strncpy(stRecord.Fspid, m_params.getString("spid").c_str(), sizeof(stRecord.Fspid) - 1);  
	strncpy(stRecord.Ffund_name, m_params.getString("fund_name").c_str(), sizeof(stRecord.Ffund_name)-1);
    strncpy(stRecord.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(stRecord.Ffund_code)-1);
	stRecord.Ftotal_fee = m_params.getLong("total_fee");
	strncpy(stRecord.Fcreate_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fcreate_time)-1);
    strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);
    stRecord.Fstate = PREPAY_INIT;	
    strncpy(stRecord.Fstandby3, m_params.getString("channel_id").c_str(), sizeof(stRecord.Fstandby3)-1);
	insertFundPrepay(m_pFundCon, stRecord);
}

/**
  * 生成基金购买记录，状态: 等待付款
  */
void FundPrepayReq::RecordFundTrade()
{
	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));

    strncpy(stRecord.Flistid, m_params.getString("fund_trans_id").c_str(), sizeof(stRecord.Flistid)-1);
    strncpy(stRecord.Fspid, m_params.getString("spid").c_str(), sizeof(stRecord.Fspid)-1);
    //strncpy(stRecord.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(stRecord.Ftrade_id)-1);
    //strncpy(stRecord.Ffund_name, m_params.getString("fund_name").c_str(), sizeof(stRecord.Ffund_name)-1);
    strncpy(stRecord.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(stRecord.Ffund_code)-1);
    stRecord.Fpur_type = PURTYPE_PURCHASE;
    stRecord.Ftotal_fee = m_params.getLong("total_fee");
    stRecord.Fstate = CREATE_INIT;
    stRecord.Flstate = LSTATE_VALID;
    strncpy(stRecord.Fchannel_id, m_params.getString("channel_id").c_str(), sizeof(stRecord.Fchannel_id)-1);
    strncpy(stRecord.Fcreate_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fcreate_time)-1);
    strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);
    stRecord.Fcur_type= querySubaccCurtype(gPtrFundDB, m_params.getString("spid"));; // 币种类型
    stRecord.Fpurpose = m_params.getInt("purpose");
    strncpy(stRecord.Fcoupon_id, m_params.getString("coupon_id").c_str(), sizeof(stRecord.Fcoupon_id)-1);
    InsertTradeFund(m_pFundCon, &stRecord);
}


/**
  * 打包输出参数
  */
void FundPrepayReq::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");

    rqst->olen = strlen(rqst->odata);
    return;
}


