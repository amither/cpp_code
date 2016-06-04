/**
  * FileName: fund_subacc_budan_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2014-01-27
  * Description: 基金交易服务 子账户补单
  */

#include "fund_commfunc.h"
#include "fund_subacc_budan_service.h"

FundSubaccBudan::FundSubaccBudan(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
	memset(&m_stTradeBuy, 0, sizeof(ST_TRADE_FUND));

}

/**
  * service step 1: 解析输入参数
  */
void FundSubaccBudan::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

	char *pMsg = (char*)(rqst->idata);

	// 读取参数
    m_params.readIntParam(pMsg, "op_type", 1,2);
	m_params.readStrParam(pMsg, "uin", 1, 64);
    m_params.readStrParam(pMsg, "fund_trans_id", 1,32);
	m_params.readStrParam(pMsg, "watch_word", 1,32);
	m_params.readStrParam(pMsg, "client_ip", 1, 16);
	
	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

	m_optype = m_params.getInt("op_type");

}


/**
  * 检查参数，获取内部参数
  */
void FundSubaccBudan::CheckParams() throw (CException)
{
	if("e10adc3949ba59abbe56e057f20f883e" != m_params.getString("watch_word"))
	{
		throw EXCEPTION(ERR_BAD_PARAM, "watch_word error"); 
	}
	
}

/**
  * 执行申购请求
  */
void FundSubaccBudan::excute() throw (CException)
{

        CheckParams();

		/* 检查基金账户记录 */
		CheckFundBind();

        /* 子账户补单 */
        SubaccBudan();

}

/*
 * 查询基金账户是否存在
 */
void FundSubaccBudan::CheckFundBind() throw (CException)
{
    if(!QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);	
    }

	//账户基金信息不一样，直接打退款标记
	if( m_fund_bind.Fuid == 0 )
	{
		throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);	
	}
}

void FundSubaccBudan::SubaccBudan() throw (CException)
{
	if(1== m_optype)
	{
		//申购
		/* 查询基金交易记录 */
        CheckFundTrade();
	}
	else if(2== m_optype)
	{
		//收益
		CheckProfitRecord();
	}
	else
	{
		EXCEPTION(ERR_BAD_PARAM, "op_type invalid");
	}

	//检查通过补子账户
	doSave();
}


/**
  * 检查基金交易记录是否已经生成,检查订单前置状态
  */
void FundSubaccBudan::CheckFundTrade() throw (CException)
{
	// 没有购买记录，报错
	if(!QueryTradeFund(m_pFundCon, m_params.getString("fund_trans_id").c_str(), PURTYPE_BUY, &m_stTradeBuy, false))
	{
		gPtrAppLog->error("buy record not exist, listid[%s]  ", m_params.getString("fund_trans_id").c_str());
		throw CException(ERR_BUYPAY_NOLIST, "buy record not exist! ", __FILE__, __LINE__);
	}

	// 物理状态无效，报错
	if(LSTATE_INVALID == m_stTradeBuy.Flstate)
	{
		gPtrAppLog->error("fund buy record, lstate is invalid. listid[%s], uid[%d] ", m_stTradeBuy.Flistid, m_stTradeBuy.Fuid);
		throw CException(ERR_TRADE_INVALID, "fund buy record, lstate is invalid. ", __FILE__, __LINE__);
	}

	//如果交易记录不是支付成功或申购确认完成，不能补单
	if(PAY_OK != m_stTradeBuy.Fstate && PURCHASE_SUC != m_stTradeBuy.Fstate )
	{
		//数据库状态不能更新
		throw CException(ERR_BUY_NOT_SUC, "fund purchase record is not success! ", __FILE__, __LINE__);
	}

	if(m_stTradeBuy.Fuid != m_fund_bind.Fuid)
	{
		TRACE_ERROR("uid in m_stTradeBuy=%d diff with in m_fund_bind=%d", 
		    m_stTradeBuy.Fuid, m_fund_bind.Fuid);
		throw CException(ERR_BASE_INFO_DIFF, "uid is different", __FILE__, __LINE__);	
	}
	FundSpConfig m_fundSpConfig;
	memset(&m_fundSpConfig, 0, sizeof(FundSpConfig));
	strncpy(m_fundSpConfig.Fspid,m_stTradeBuy.Fspid, sizeof(m_fundSpConfig.Fspid) - 1);
	checkFundSpAndFundcode(m_pFundCon,m_fundSpConfig,false);

	//非实时确认份额的交易单，spe_tag = 4 不可以补单
	if(m_fundSpConfig.Fbuy_confirm_type==SPCONFIG_BALANCE_T1_CONFIRM && m_stTradeBuy.Fspe_tag == TRADE_SPETAG_UNITS_UNUSABLE)
	{
		//数据库状态不能更新
		throw CException(ERR_BUY_NOT_SUC, "fund purchase record spe_tag is  unusable! ", __FILE__, __LINE__);
	}
	
       //非实时确认份额的交易单，状态为3才可以补单
	if(m_fundSpConfig.Fbuy_confirm_type==SPCONFIG_BALANCE_T1_CONFIRM &&  PURCHASE_SUC != m_stTradeBuy.Fstate )
	{
		//数据库状态不能更新
		throw CException(ERR_BUY_NOT_SUC, "fund purchase record state is not success! ", __FILE__, __LINE__);
	}

	
	// 得到账户相关信息
	m_params.setParam("sub_trans_id", m_stTradeBuy.Fsub_trans_id);//用于更新子账户
	m_params.setParam("total_fee", m_stTradeBuy.Ftotal_fee);
	m_params.setParam("spid", m_stTradeBuy.Fspid);

	if(m_fundSpConfig.Fbuy_confirm_type == SPCONFIG_BALANCE_T1_CONFIRM && m_stTradeBuy.Fspe_tag!= TRADE_SPETAG_UNITS_UNUSABLE)
	{
              m_params.setParam("total_fee", m_stTradeBuy.Freal_redem_amt);
	}

	 if(m_params.getLong("total_fee") <= 0)
	 {
		throw CException(ERR_BAD_PARAM, "fund buy pay, total_fee<=0!", __FILE__, __LINE__);
	 }

}

void FundSubaccBudan::CheckProfitRecord() throw (CException)
{
	FundProfitRecord  stRecord;
    memset(&stRecord, 0, sizeof(FundProfitRecord));
                    
    strncpy(stRecord.Flistid, m_params.getString("fund_trans_id").c_str(), sizeof(stRecord.Flistid)-1);
	strncpy(stRecord.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(stRecord.Ftrade_id)-1);

	if(!queryFundProfitRecord(m_pFundCon, stRecord, false))
	{
		throw CException(ERR_PROFIT_NOT_EXIST, "profit record not exist! ", __FILE__, __LINE__);
	}

	if(!(0 == strcmp(stRecord.Ftrade_id, m_fund_bind.Ftrade_id)))
	{
		throw CException(ERR_BASE_INFO_DIFF, "trade_id is different", __FILE__, __LINE__);	
	}

	if(FUND_PROFIT_NORMAL != stRecord.Fprofit_type && FUND_PROFIT_REWARD != stRecord.Fprofit_type)
	{
		throw CException(ERR_BASE_INFO_DIFF, "扫尾收益不可以补单", __FILE__, __LINE__);	
	}
	if(stRecord.Fprofit-stRecord.Fend_tail_fee<=0)
	{
		throw CException(ERR_BASE_INFO_DIFF, "需要增加子账户的收益(profit-end_tail_fee)小于等于0", __FILE__, __LINE__);	
	}

	// 得到账户相关信息
	m_params.setParam("sub_trans_id", stRecord.Fsub_trans_id);//用于更新子账户
	m_params.setParam("total_fee", stRecord.Fprofit-stRecord.Fend_tail_fee); //收益
	m_params.setParam("spid", stRecord.Fspid);
}

/**
 * 子账户充值
 */
void FundSubaccBudan::doSave() throw(CException)
{
	gPtrAppLog->normal("doSave, listid[%s]  ", m_params.getString("sub_trans_id").c_str());
	
	string subaccTime = m_params.getString("systime");
	string desc = (m_optype == 1) ? "基金申购-补单" : "基金收益-补单";
	
	int iResult = SubaccSave(gPtrSubaccRpc, m_params.getString("spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
		m_params.getString("sub_trans_id"), m_params.getLong("total_fee"), desc, subaccTime, m_optype);

	//未开通子账户，补开
	if(ERR_SUBACC_NOT_EXIST == iResult)
	{
		createSubaccUser(m_request, m_params.getString("spid"), m_params.getString("uin"), m_params.getString("client_ip"));

		//开户后重做
		iResult = SubaccSave(gPtrSubaccRpc, m_params.getString("spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_params.getString("sub_trans_id"), m_params.getLong("total_fee"), desc, subaccTime, m_optype);
	}

	if(0 != iResult)
	{
		throw CException(iResult, "doSave subacc exception.", __FILE__, __LINE__);
	}
				
}


/**
  * 打包输出参数
  */
void FundSubaccBudan::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");

    rqst->olen = strlen(rqst->odata);
    return;
}


