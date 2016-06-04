/**
  * FileName: fund_prepay_ack_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-11-07
  * Description: 基金交易服务 非实名认证用户(暂无法开通理财账户)预支付确认接口
  */

#include "fund_commfunc.h"
#include "fund_prepay_ack_service.h"

FundPrepayAck::FundPrepayAck(CMySQL* mysql)
{
    m_pFundCon = mysql;

	memset(&m_fund_prepay, 0, sizeof(FundPrepay));    
	memset(&m_stTradeBuy, 0, sizeof(ST_TRADE_FUND));

}

/**
  * service step 1: 解析输入参数
  */
void FundPrepayAck::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};
	
    char *pMsg = (char*)(rqst->idata);

	m_params.readStrParam(pMsg, "uin", 0, 64);
	//m_params.readIntParam(pMsg, "uid", 10000,MAX_INTEGER);
	m_params.readStrParam(pMsg, "fund_trans_id", 1, 32);
	m_params.readStrParam(pMsg, "cft_trans_id", 1, 32);
	m_params.readStrParam(pMsg, "spid", 10, 15);
	//m_params.readLongParam(pMsg, "total_fee", 1, MAX_LONG);
	m_params.readIntParam(pMsg, "op_type", 1, 2);
	//m_params.readStrParam(szMsg, "client_ip", 0, 16);
    m_params.readStrParam(pMsg, "token", 1, 32);   // 接口token
	

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

}


/*
 * 生成基金注册用token
 */
string FundPrepayAck::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照uid|fund_trans_id|spid|sp_billno|total_fee|key
    // 规则生成原串
    ss << m_params["uin"] << "|" ;
    ss << m_params["uid"] << "|" ;
    ss << m_params["fund_trans_id"] << "|" ;
	ss << m_params["cft_trans_id"] << "|" ;
    ss << m_params["spid"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * 检验token
 */
void FundPrepayAck::CheckToken() throw (CException)
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
void FundPrepayAck::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();

}

/**
  * 执行申购请求
  */
void FundPrepayAck::excute() throw (CException)
{
    try
    {
        CheckParams();

         /* 开启事务 */
        m_pFundCon->Begin();
		 
		CheckFundPrepay();

		UpdateFundPrepay();

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
void FundPrepayAck::CheckFundPrepay() throw (CException)
{
	strncpy(m_fund_prepay.Flistid, m_params.getString("fund_trans_id").c_str(), sizeof(m_fund_prepay.Flistid) - 1);
	
    if(!queryFundPrepay(m_pFundCon, m_fund_prepay, true))
	{
		gPtrAppLog->error("prepay record not exist, listid[%s]  ", m_params.getString("fund_trans_id").c_str());
		throw CException(ERR_PREPAY_NOT_EXIST, "prepay record not exist! ", __FILE__, __LINE__);
	}

	// 检查关键参数
    if( (0 != strcmp(m_fund_prepay.Fspid, m_params.getString("spid").c_str())))
    {
        TRACE_ERROR("prepay record exists, spid is different! spid in db[%s], spid input[%s]", 
			m_fund_prepay.Fspid, m_params.getString("spid").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "prepay record exists, spid is different!", __FILE__, __LINE__);
    }

    if(!m_params.getString("uin").empty() && 0 != strcmp(m_fund_prepay.Fuin, m_params.getString("uin").c_str()))
    {
        TRACE_ERROR("prepay record exists, uin is different! uin in db[%s], uin input[%s] ", 
			m_fund_prepay.Fuin, m_params.getString("uin").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "prepay record exists, uin is different!", __FILE__, __LINE__);
    }

	// 检查重入
	if(m_params.getInt("op_type")==m_fund_prepay.Fauthen_state&&
		0==strcmp(m_params.getString("fund_trans_id").c_str(),m_fund_prepay.Flistid)&&
		0==strcmp(m_params.getString("cft_trans_id").c_str(),m_fund_prepay.Fcft_trans_id))
	{
		throw EXCEPTION(ERR_REPEAT_ENTRY, "repeat entry");
	}

}

void FundPrepayAck::UpdateFundPrepay()
{
	
	if(m_params.getInt("op_type") == OP_TYPE_AUTHEN_OK || m_params.getInt("op_type") == OP_TYPE_AUTHEN_FAIL)
	{
		UpdatePrepayForAuthen();
	}
	/*
	else if(m_params.getInt("op_type") == OP_TYPE_PREPAY_SUC)
	{
		UpdatePrepayForPayOk();
	}
	else if(m_params.getInt("op_type") == OP_TYPE_PREPAY_REFUND)
	{
		UpdatePrepayForRefund();
	}
	*/
	else
	{
		throw EXCEPTION(ERR_BAD_PARAM, "input op_type error");    
	}

}


void FundPrepayAck::UpdatePrepayForAuthen()
{
	FundPrepay stRecord;
	memset(&stRecord, 0, sizeof(FundPrepay));
	
	strncpy(stRecord.Flistid, m_params.getString("fund_trans_id").c_str(), sizeof(stRecord.Flistid) - 1);
	strncpy(stRecord.Fcft_trans_id, m_params.getString("cft_trans_id").c_str(), sizeof(stRecord.Fcft_trans_id) - 1);
	strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);
	//有可能首次MQ认证实名失败，cgi配置了白名单，第二次认证实名认为是成功的
	stRecord.Fauthen_state= m_params.getInt("op_type");

	updateFundPrepay(m_pFundCon, stRecord);
}
void FundPrepayAck::UpdatePrepayForPayOk()
{
	// 没有购买记录，报错
	if(!QueryTradeFund(m_pFundCon, m_params.getString("fund_trans_id").c_str(), PURTYPE_PURCHASE, &m_stTradeBuy, true))
	{
		gPtrAppLog->error("buy record not exist, listid[%s]  ", m_params.getString("fund_trans_id").c_str());
		throw CException(ERR_BUYPAY_NOLIST, "buy record not exist! ", __FILE__, __LINE__);
	}
	
	//更新支付成功
	if(PREPAY_OK == m_fund_prepay.Fstate)
	{
		//重入返回
		throw CException(ERR_REPEAT_ENTRY, "the prepay record has pay! ", __FILE__, __LINE__);
	}

	//非初始状态的不能在更新成功，重入也不能报重入错误，因为订单已经转入退款，只能等批跑退款，不能在进行任何操作
	if(PREPAY_INIT != m_fund_prepay.Fstate)
	{
		throw CException(ERR_PREPAY_CANNOT_UPDATE, "the prepay connot be update to pay success! ", __FILE__, __LINE__);
	}

	FundPrepay stRecord;
	memset(&stRecord, 0, sizeof(FundPrepay));
	
	strncpy(stRecord.Flistid, m_params.getString("fund_trans_id").c_str(), sizeof(stRecord.Flistid) - 1);
	stRecord.Fuid = m_params.getInt("uid");
	strncpy(stRecord.Fcft_trans_id, m_params.getString("cft_trans_id").c_str(), sizeof(stRecord.Fcft_trans_id) - 1);  
    strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);
    stRecord.Fstate = PREPAY_OK;
	
	updateFundPrepay(m_pFundCon, stRecord);
}
void FundPrepayAck::UpdatePrepayForRefund()
{
	//更新为退款
	if(PREPAY_REFUND_INIT == m_fund_prepay.Fstate || PREPAY_REFUND_SUC == m_fund_prepay.Fstate)
	{
		//重入返回
		throw CException(ERR_REPEAT_ENTRY, "the prepay record has refund! ", __FILE__, __LINE__);
	}
	
	if(PREPAY_OK != m_fund_prepay.Fstate)
	{
		//非支付成功不能退款
		throw CException(ERR_PREPAY_CANNOT_UPDATE, "the prepay connot be update to pay success! ", __FILE__, __LINE__);
	}

	//检查该交易单是否创建申购单,交易单不存在或者申请申购失败才能退款，否则不允许退款
	ST_TRADE_FUND  trade_fund;
    memset(&trade_fund, 0, sizeof(ST_TRADE_FUND));
	if(QueryTradeFund(m_pFundCon, m_params.getString("fund_trans_id").c_str(), PURTYPE_PURCHASE, &trade_fund, true))
	{
		if(PUR_REQ_FAIL != trade_fund.Fstate)
		{
			gPtrAppLog->error("fund trade record not purchase request fail cannot refund,listid[%s]", trade_fund.Flistid);
			throw CException(ERR_PREPAY_CANNOT_REFUND, "fund buy pay, lstate is invalid. ", __FILE__, __LINE__);
		}
	}

	FundPrepay stRecord;
	memset(&stRecord, 0, sizeof(FundPrepay));
	
	strncpy(stRecord.Flistid, m_params.getString("fund_trans_id").c_str(), sizeof(stRecord.Flistid) - 1);
	strncpy(stRecord.Frefund_id, m_params.getString("fund_trans_id").c_str(), sizeof(stRecord.Frefund_id) - 1);//TODO
	stRecord.Fstate = PREPAY_REFUND_INIT;
	strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);

	updateFundPrepay(m_pFundCon, stRecord);
}


/**
  * 打包输出参数
  */
void FundPrepayAck::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
	CUrlAnalyze::setParam(rqst->odata, "acct_type", m_fund_prepay.Facct_type);
	CUrlAnalyze::setParam(rqst->odata, "fund_code", m_fund_prepay.Ffund_code);
	CUrlAnalyze::setParam(rqst->odata, "total_fee", toString(m_fund_prepay.Ftotal_fee).c_str());
	CUrlAnalyze::setParam(rqst->odata, "openidA", m_fund_prepay.Fopenid);
	CUrlAnalyze::setParam(rqst->odata, "uin", m_fund_prepay.Fuin);
    CUrlAnalyze::setParam(rqst->odata, "channel_id", m_fund_prepay.Fstandby3);
    rqst->olen = strlen(rqst->odata);
    return;
}


