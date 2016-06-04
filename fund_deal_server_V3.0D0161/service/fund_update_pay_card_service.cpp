/**
  * FileName: fund_update_pay_card_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2014-01-09
  * Description: 基金交易服务 更新用户的支付卡信息 源文件
  * 该接口为服务调用，不部署在relay上
  */

#include "fund_commfunc.h"
#include "fund_update_pay_card_service.h"

FundUpdatePayCard::FundUpdatePayCard(CMySQL* mysql)
{
    m_pFundCon = mysql;                 

}

/**
  * service step 1: 解析输入参数
  */
void FundUpdatePayCard::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_update_pay_card_service] receives: %s", szMsg);

	m_params.readStrParam(szMsg, "uin", 1, 64);
    m_params.readStrParam(szMsg, "desc", 0, 128);
    m_params.readStrParam(szMsg, "client_ip", 0, 16);
    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

}

/*
 * 生成基金注册用token
 */
string FundUpdatePayCard::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照uin|key
    // 规则生成原串
    ss << m_params["uin"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * 检验token
 */
void FundUpdatePayCard::CheckToken() throw (CException)
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
void FundUpdatePayCard::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();
}

/**
  * 执行申购请求
  */
void FundUpdatePayCard::excute() throw (CException)
{
	try
	{
	    CheckParams();

	     /* 开启事务 */
	    m_pFundCon->Begin();
		 
		 /* 更新用户支付卡 */
		UpdatePayCard();

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


void FundUpdatePayCard::UpdatePayCard()
{
	ST_FUND_BIND fund_bind; 
	memset(&fund_bind, 0, sizeof(ST_FUND_BIND));
	if(!QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &fund_bind, false))
	{
		TRACE_WARN("query fund bind not exist,uin=[%s]", m_params.getString("uin").c_str());	
		return;
	}

	// 检查总资产,包含理财通余额
	LONG balance = queryUserTotalAsset(fund_bind.Fuid,fund_bind.Ftrade_id);
	if(balance != 0 && gPtrConfig->m_AppCfg.update_pay_card_limit < balance)
	{
		//打印日志，但是不报错，也不更新绑定卡
		TRACE_WARN("user balance not less limit,total balance=[%zd],limit=[%zd]", balance, gPtrConfig->m_AppCfg.update_pay_card_limit);	
		return ;
	}

	//不能有赎回中的交易单
	if (isUserExistRedemingRecords(gPtrFundDB,fund_bind.Fuid) == true)
	{
		TRACE_WARN("user has redeming records");	
		return ;
	}
	
	// 检查是否存在未确认资产
	vector < FUND_UNCONFIRM > unconfirmVec;
	//TODO:fundUnconfirm 删除逻辑:第三步删除unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
		int unconfirmCount = queryValidFundUnconfirmByTradeId(gPtrFundDB,fund_bind.Ftrade_id,unconfirmVec);
		if(unconfirmCount > 0)
		{
			TRACE_WARN("user has unconfirm records");	
			return ;
		}
	}
	if(queryUnfinishTransExists(gPtrFundDB,fund_bind.Ftrade_id))
	{
        TRACE_WARN("checkUnfinish Index trans record exist!");
		return ;
	}

	FundPayCard old_pay_card;
	memset(&old_pay_card, 0, sizeof(FundPayCard));
	strncpy(old_pay_card.Fqqid, m_params.getString("uin").c_str(), sizeof(old_pay_card.Fqqid) - 1);

	if(!queryFundPayCard(m_pFundCon,old_pay_card,true))
	{
		return;
	}

	//把支付卡信息都清空
	FundPayCard fund_pay_card;
	memset(&fund_pay_card, 0, sizeof(fund_pay_card));
	strncpy(fund_pay_card.Fqqid, m_params.getString("uin").c_str(), sizeof(fund_pay_card.Fqqid) - 1);
	strncpy(fund_pay_card.Fmodify_time,  m_params.getString("systime").c_str(), sizeof(fund_pay_card.Fmodify_time) - 1);
    //补齐刷新Fsign需要的字段
    strncpy(fund_pay_card.Ftrade_id, old_pay_card.Ftrade_id, sizeof(fund_pay_card.Ftrade_id) - 1);
    
	updateFundPayCard(m_pFundCon, fund_pay_card);
	//setPayCardToKV(m_pFundCon, fund_pay_card);
	//直接将ckv 数据删除了
	delPayCardToKV(m_params.getString("uin"));
	

}


/**
  * 打包输出参数
  */
void FundUpdatePayCard::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");

    rqst->olen = strlen(rqst->odata);
    return;
}


